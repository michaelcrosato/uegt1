#include "World/UEGT1BiomeTile.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/CollisionProfile.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "World/UEGT1Palette.h"
#include "World/UEGT1WorldLayout.h"

AUEGT1BiomeTile::AUEGT1BiomeTile()
{
	PrimaryActorTick.bCanEverTick = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeFinder(TEXT("/Engine/BasicShapes/Cone.Cone"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	ShapeMaterial = MaterialFinder.Object;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	GroundInstances = CreateInstanceComponent(TEXT("GroundInstances"), CubeFinder.Object, true);
	GroundInstances->SetCullDistances(0, 0);
	TrailInstances = CreateInstanceComponent(TEXT("TrailInstances"), CubeFinder.Object, false);
	TrunkInstances = CreateInstanceComponent(TEXT("TrunkInstances"), CylinderFinder.Object, true);
	CanopyInstances = CreateInstanceComponent(TEXT("CanopyInstances"), ConeFinder.Object, false);
	RockInstances = CreateInstanceComponent(TEXT("RockInstances"), CubeFinder.Object, true);
	GrassInstances = CreateInstanceComponent(TEXT("GrassInstances"), ConeFinder.Object, false);
	GrassInstances->SetCullDistances(2500, 7200);
}

UHierarchicalInstancedStaticMeshComponent* AUEGT1BiomeTile::CreateInstanceComponent(const FName Name, UStaticMesh* Mesh, bool bCollision)
{
	UHierarchicalInstancedStaticMeshComponent* Component = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(Name);
	Component->SetupAttachment(SceneRoot);
	Component->SetStaticMesh(Mesh);
	Component->SetMobility(EComponentMobility::Static);
	Component->SetCollisionEnabled(bCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	Component->SetCollisionProfileName(bCollision ? UCollisionProfile::BlockAll_ProfileName : UCollisionProfile::NoCollision_ProfileName);
	Component->SetCanEverAffectNavigation(false);
	Component->SetCullDistances(9000, 18000);
	return Component;
}

void AUEGT1BiomeTile::InitializeTile(FIntPoint InTileCoordinate, int32 InWorldSeed)
{
	TileCoordinate = InTileCoordinate;
	WorldSeed = InWorldSeed;
	SetActorLocation(FVector(TileCoordinate.X * TileSize, TileCoordinate.Y * TileSize, 0.0f));
#if WITH_EDITOR
	SetActorLabel(FString::Printf(TEXT("BiomeTile_%+d_%+d"), TileCoordinate.X, TileCoordinate.Y));
#endif
	RebuildTile();
}

void AUEGT1BiomeTile::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RebuildTile();
}

void AUEGT1BiomeTile::BeginPlay()
{
	Super::BeginPlay();
	RebuildTile();
}

void AUEGT1BiomeTile::RebuildTile()
{
	GroundInstances->ClearInstances();
	TrailInstances->ClearInstances();
	TrunkInstances->ClearInstances();
	CanopyInstances->ClearInstances();
	RockInstances->ClearInstances();
	GrassInstances->ClearInstances();

	const uint32 Seed = HashCombine(GetTypeHash(WorldSeed), HashCombine(GetTypeHash(TileCoordinate.X), GetTypeHash(TileCoordinate.Y)));
	FRandomStream Random(static_cast<int32>(Seed));
	GenerateGroundAndTrails(Random);
	GenerateVegetation(Random);
	GenerateBoundaryCliffs(Random);

	const float TileTint = ((TileCoordinate.X + TileCoordinate.Y) & 1) == 0 ? 1.0f : 0.91f;
	AssignColorMaterial(GroundInstances, UEGT1Palette::Moss * TileTint);
	AssignColorMaterial(TrailInstances, UEGT1Palette::Path);
	AssignColorMaterial(TrunkInstances, UEGT1Palette::Bark);
	AssignColorMaterial(CanopyInstances, UEGT1Palette::Fern * TileTint);
	AssignColorMaterial(RockInstances, UEGT1Palette::Stone * (0.88f + Random.FRandRange(0.0f, 0.12f)));
	AssignColorMaterial(GrassInstances, UEGT1Palette::Fern * 1.08f);
}

void AUEGT1BiomeTile::GenerateGroundAndTrails(FRandomStream& Random)
{
	GroundInstances->AddInstance(FTransform(FRotator::ZeroRotator, GetActorLocation() + FVector(0.0f, 0.0f, -110.0f),
		FVector(TileSize / 100.0f, TileSize / 100.0f, 2.2f)), true);

	for (const FVector& WaystoneLocation : UEGT1WorldLayout::GetWaystoneLocations())
	{
		const FVector Direction = WaystoneLocation - UEGT1WorldLayout::SanctuaryLocation;
		const float Distance = Direction.Size2D();
		const int32 StepCount = FMath::CeilToInt(Distance / 430.0f);
		for (int32 Step = 1; Step < StepCount; ++Step)
		{
			const float Alpha = static_cast<float>(Step) / StepCount;
			FVector WorldPoint = FMath::Lerp(UEGT1WorldLayout::SanctuaryLocation, WaystoneLocation, Alpha);
			WorldPoint += FVector(Random.FRandRange(-45.0f, 45.0f), Random.FRandRange(-45.0f, 45.0f), 0.0f);
			if (!IsInsideTile(WorldPoint, 80.0f))
			{
				continue;
			}
			const float Yaw = Direction.Rotation().Yaw + Random.FRandRange(-8.0f, 8.0f);
			TrailInstances->AddInstance(FTransform(FRotator(0.0f, Yaw, 0.0f), WorldPoint + FVector(0.0f, 0.0f, 7.0f),
				FVector(Random.FRandRange(1.2f, 1.8f), Random.FRandRange(0.65f, 1.0f), 0.12f)), true);
		}
	}
}

void AUEGT1BiomeTile::GenerateVegetation(FRandomStream& Random)
{
	const float HalfTile = TileSize * 0.5f;
	for (int32 Index = 0; Index < TreesPerTile; ++Index)
	{
		const FVector LocalPosition(Random.FRandRange(-HalfTile + 180.0f, HalfTile - 180.0f), Random.FRandRange(-HalfTile + 180.0f, HalfTile - 180.0f), 0.0f);
		const FVector WorldPosition = GetActorLocation() + LocalPosition;
		if (UEGT1WorldLayout::IsReservedGameplaySpace(WorldPosition, 150.0f))
		{
			continue;
		}

		const float Height = Random.FRandRange(330.0f, 520.0f);
		const float Width = Random.FRandRange(0.75f, 1.3f);
		const float Yaw = Random.FRandRange(0.0f, 360.0f);
		TrunkInstances->AddInstance(FTransform(FRotator(0.0f, Yaw, 0.0f), WorldPosition + FVector(0.0f, 0.0f, Height * 0.42f),
			FVector(0.26f * Width, 0.26f * Width, Height * 0.0084f)), true);
		CanopyInstances->AddInstance(FTransform(FRotator(0.0f, Yaw, 0.0f), WorldPosition + FVector(0.0f, 0.0f, Height * 0.92f),
			FVector(1.35f * Width, 1.35f * Width, Height * 0.009f)), true);
	}

	for (int32 Index = 0; Index < RocksPerTile; ++Index)
	{
		const FVector LocalPosition(Random.FRandRange(-HalfTile, HalfTile), Random.FRandRange(-HalfTile, HalfTile), Random.FRandRange(20.0f, 60.0f));
		const FVector WorldPosition = GetActorLocation() + LocalPosition;
		if (UEGT1WorldLayout::IsReservedGameplaySpace(WorldPosition, 80.0f))
		{
			continue;
		}
		RockInstances->AddInstance(FTransform(FRotator(Random.FRandRange(-18.0f, 18.0f), Random.FRandRange(0.0f, 360.0f), Random.FRandRange(-18.0f, 18.0f)),
			WorldPosition, FVector(Random.FRandRange(0.45f, 1.2f), Random.FRandRange(0.4f, 1.0f), Random.FRandRange(0.35f, 0.85f))), true);
	}

	for (int32 Index = 0; Index < GrassClustersPerTile; ++Index)
	{
		const FVector LocalPosition(Random.FRandRange(-HalfTile, HalfTile), Random.FRandRange(-HalfTile, HalfTile), 30.0f);
		const FVector WorldPosition = GetActorLocation() + LocalPosition;
		if (!UEGT1WorldLayout::IsReservedGameplaySpace(WorldPosition, -100.0f))
		{
			GrassInstances->AddInstance(FTransform(FRotator(0.0f, Random.FRandRange(0.0f, 360.0f), 0.0f), WorldPosition,
				FVector(Random.FRandRange(0.12f, 0.25f), Random.FRandRange(0.12f, 0.25f), Random.FRandRange(0.35f, 0.7f))), true);
		}
	}
}

void AUEGT1BiomeTile::GenerateBoundaryCliffs(FRandomStream& Random)
{
	const int32 Radius = UEGT1WorldLayout::TileRadius;
	if (FMath::Abs(TileCoordinate.X) != Radius && FMath::Abs(TileCoordinate.Y) != Radius)
	{
		return;
	}

	const float HalfTile = TileSize * 0.5f;
	for (int32 Index = 0; Index < 7; ++Index)
	{
		FVector Position;
		if (FMath::Abs(TileCoordinate.X) == Radius)
		{
			Position.X = FMath::Sign(TileCoordinate.X) * (HalfTile - Random.FRandRange(80.0f, 260.0f));
			Position.Y = Random.FRandRange(-HalfTile, HalfTile);
		}
		else
		{
			Position.X = Random.FRandRange(-HalfTile, HalfTile);
			Position.Y = FMath::Sign(TileCoordinate.Y) * (HalfTile - Random.FRandRange(80.0f, 260.0f));
		}
		const float HeightScale = Random.FRandRange(3.5f, 8.0f);
		Position.Z = HeightScale * 50.0f - 15.0f;
		RockInstances->AddInstance(FTransform(FRotator(Random.FRandRange(-12.0f, 12.0f), Random.FRandRange(0.0f, 360.0f), Random.FRandRange(-12.0f, 12.0f)),
			GetActorLocation() + Position, FVector(Random.FRandRange(2.2f, 4.5f), Random.FRandRange(2.2f, 4.5f), HeightScale)), true);
	}
}

void AUEGT1BiomeTile::AssignColorMaterial(UHierarchicalInstancedStaticMeshComponent* Component, const FLinearColor& Color)
{
	if (!ShapeMaterial || !Component)
	{
		return;
	}
	UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(ShapeMaterial, this);
	Material->SetVectorParameterValue(TEXT("Color"), Color);
	Component->SetMaterial(0, Material);
}

bool AUEGT1BiomeTile::IsInsideTile(const FVector& WorldPosition, float Padding) const
{
	const FVector Local = WorldPosition - GetActorLocation();
	const float HalfTile = TileSize * 0.5f - Padding;
	return FMath::Abs(Local.X) <= HalfTile && FMath::Abs(Local.Y) <= HalfTile;
}

int32 AUEGT1BiomeTile::GetGeneratedInstanceCount() const
{
	return GroundInstances->GetInstanceCount() + TrailInstances->GetInstanceCount() + TrunkInstances->GetInstanceCount() +
		CanopyInstances->GetInstanceCount() + RockInstances->GetInstanceCount() + GrassInstances->GetInstanceCount();
}
