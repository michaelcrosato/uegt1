#include "World/UEGT1BiomeTile.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/CollisionProfile.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Settings/UEGT1GameUserSettings.h"
#include "UObject/ConstructorHelpers.h"
#include "World/UEGT1Palette.h"
#include "World/UEGT1WorldLayout.h"

AUEGT1BiomeTile::AUEGT1BiomeTile()
{
	PrimaryActorTick.bCanEverTick = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeFinder(TEXT("/Engine/BasicShapes/Cone.Cone"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	ShapeMaterial = MaterialFinder.Object;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SceneRoot->SetMobility(EComponentMobility::Static);
	SetRootComponent(SceneRoot);
	GroundInstances = CreateInstanceComponent(TEXT("GroundInstances"), CubeFinder.Object, true);
	GroundInstances->SetCullDistances(0, 0);
	TrailInstances = CreateInstanceComponent(TEXT("TrailInstances"), CubeFinder.Object, false);
	TrunkInstances = CreateInstanceComponent(TEXT("TrunkInstances"), CylinderFinder.Object, true);
	ConiferInstances = CreateInstanceComponent(TEXT("ConiferInstances"), ConeFinder.Object, false);
	BroadleafInstances = CreateInstanceComponent(TEXT("BroadleafInstances"), SphereFinder.Object, false);
	RockInstances = CreateInstanceComponent(TEXT("RockInstances"), CubeFinder.Object, true);
	GrassInstances = CreateInstanceComponent(TEXT("GrassInstances"), ConeFinder.Object, false);
	CropInstances = CreateInstanceComponent(TEXT("CropInstances"), CubeFinder.Object, false);
	WaterInstances = CreateInstanceComponent(TEXT("WaterInstances"), CubeFinder.Object, false);
	WaveInstances = CreateInstanceComponent(TEXT("WaveInstances"), CubeFinder.Object, false);
	PeakInstances = CreateInstanceComponent(TEXT("PeakInstances"), ConeFinder.Object, false);
	GrassInstances->SetCullDistances(2500, 9000);
	CropInstances->SetCullDistances(3500, 12000);
	WaveInstances->SetCullDistances(4500, 18000);
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
	Component->SetCullDistances(10000, 30000);
	return Component;
}

void AUEGT1BiomeTile::InitializeTile(FIntPoint InTileCoordinate, int32 InWorldSeed)
{
	TileCoordinate = InTileCoordinate;
	WorldSeed = InWorldSeed;
	TileSize = UEGT1WorldLayout::GetTileSize();
	SetActorLocation(FVector(TileCoordinate.X * TileSize, TileCoordinate.Y * TileSize, 0.0f));
#if WITH_EDITOR
	SetActorLabel(FString::Printf(TEXT("RegionTile_%+d_%+d"), TileCoordinate.X, TileCoordinate.Y));
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
	UUEGT1GameUserSettings::OnGraphicsSettingsApplied.AddUObject(this, &AUEGT1BiomeTile::ApplyGraphicsSettings);
	ApplyGraphicsSettings();
}

void AUEGT1BiomeTile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UUEGT1GameUserSettings::OnGraphicsSettingsApplied.RemoveAll(this);
	Super::EndPlay(EndPlayReason);
}

void AUEGT1BiomeTile::RebuildTile()
{
	for (UHierarchicalInstancedStaticMeshComponent* Component : { GroundInstances, TrailInstances, TrunkInstances, ConiferInstances,
		BroadleafInstances, RockInstances, GrassInstances, CropInstances, WaterInstances, WaveInstances, PeakInstances })
	{
		Component->ClearInstances();
	}

	const uint32 Seed = HashCombine(GetTypeHash(WorldSeed), HashCombine(GetTypeHash(TileCoordinate.X), GetTypeHash(TileCoordinate.Y)));
	FRandomStream Random(static_cast<int32>(Seed));
	const FUEGT1RegionSample TileSample = UEGT1WorldLayout::SampleRegion(GetActorLocation());
	DominantBiome = TileSample.GetDominantBiome();
	GenerateTerrain(Random);
	GenerateRoutes(Random);
	GenerateVegetation(Random);
	GenerateRegionalFeatures(Random);

	const float TileTint = ((TileCoordinate.X + TileCoordinate.Y) & 1) == 0 ? 1.0f : 0.96f;
	AssignColorMaterial(GroundInstances, GetBlendedGroundColor(TileSample.Biomes) * TileTint);
	AssignColorMaterial(TrailInstances, UEGT1Palette::Path * 1.22f);
	AssignColorMaterial(TrunkInstances, UEGT1Palette::Bark);
	AssignColorMaterial(ConiferInstances, FMath::Lerp(UEGT1Palette::Fern, UEGT1Palette::DeepForest,
		TileSample.Biomes.Highlands * 0.65f));
	AssignColorMaterial(BroadleafInstances, FMath::Lerp(UEGT1Palette::Fern, UEGT1Palette::Tropical,
		TileSample.Biomes.Tropical));
	AssignColorMaterial(RockInstances, FMath::Lerp(UEGT1Palette::Stone, UEGT1Palette::Sand,
		TileSample.Biomes.Coast * 0.45f));
	AssignColorMaterial(GrassInstances, FMath::Lerp(UEGT1Palette::Meadow, UEGT1Palette::Tropical,
		TileSample.Biomes.Tropical));
	AssignColorMaterial(CropInstances, FMath::Lerp(UEGT1Palette::Wheat, UEGT1Palette::Fern,
		((TileCoordinate.X + TileCoordinate.Y) & 1) == 0 ? 0.15f : 0.48f));
	AssignColorMaterial(WaterInstances, UEGT1Palette::Water);
	AssignColorMaterial(WaveInstances, UEGT1Palette::WaterHighlight);
	AssignColorMaterial(PeakInstances, UEGT1Palette::Highland * 0.82f);
}

void AUEGT1BiomeTile::GenerateTerrain(FRandomStream& Random)
{
	const int32 CellCount = FMath::Max(3, TerrainCellsPerAxis);
	const float CellSize = TileSize / CellCount;
	const float HalfTile = TileSize * 0.5f;
	const float Thickness = 520.0f;
	for (int32 Y = 0; Y < CellCount; ++Y)
	{
		for (int32 X = 0; X < CellCount; ++X)
		{
			const FVector Position = GetActorLocation() + FVector(-HalfTile + (X + 0.5f) * CellSize, -HalfTile + (Y + 0.5f) * CellSize, 0.0f);
			const FUEGT1RegionSample Sample = UEGT1WorldLayout::SampleRegion(Position);
			const float SampleOffset = CellSize * 0.42f;
			const float Left = UEGT1WorldLayout::SampleRegion(Position - FVector(SampleOffset, 0.0f, 0.0f)).SurfaceHeight;
			const float Right = UEGT1WorldLayout::SampleRegion(Position + FVector(SampleOffset, 0.0f, 0.0f)).SurfaceHeight;
			const float Back = UEGT1WorldLayout::SampleRegion(Position - FVector(0.0f, SampleOffset, 0.0f)).SurfaceHeight;
			const float Front = UEGT1WorldLayout::SampleRegion(Position + FVector(0.0f, SampleOffset, 0.0f)).SurfaceHeight;
			const FVector Normal = FVector(-(Right - Left) / (SampleOffset * 2.0f), -(Front - Back) / (SampleOffset * 2.0f), 1.0f).GetSafeNormal();
			const FQuat Rotation = FQuat::FindBetweenNormals(FVector::UpVector, Normal);
			const FVector Center = Position + FVector(0.0f, 0.0f, Sample.SurfaceHeight) - Normal * (Thickness * 0.5f);
			GroundInstances->AddInstance(FTransform(Rotation, Center, FVector(CellSize / 100.0f * 1.04f, CellSize / 100.0f * 1.04f, Thickness / 100.0f)), true);

			if (Sample.WaterDepth > 18.0f && (Sample.Biomes.Coast + Sample.Biomes.Ocean) > 0.12f)
			{
				WaterInstances->AddInstance(FTransform(FRotator::ZeroRotator,
					FVector(Position.X, Position.Y, UEGT1WorldLayout::GetSeaLevel() - 6.0f), FVector(CellSize / 100.0f * 1.04f, CellSize / 100.0f * 1.04f, 0.12f)), true);
				if (((X + Y + TileCoordinate.X + TileCoordinate.Y) & 1) == 0)
				{
					WaveInstances->AddInstance(FTransform(FRotator(0.0f, Random.FRandRange(-8.0f, 8.0f), 0.0f),
						FVector(Position.X, Position.Y, UEGT1WorldLayout::GetSeaLevel() + 2.0f), FVector(CellSize / 155.0f, 0.10f, 0.035f)), true);
				}
			}
		}
	}
}

void AUEGT1BiomeTile::GenerateRoutes(FRandomStream& Random)
{
	for (const FVector& WaystoneLocation : UEGT1WorldLayout::GetWaystoneLocations())
	{
		const FVector Direction = WaystoneLocation - UEGT1WorldLayout::GetSanctuaryLocation();
		const float Distance = Direction.Size2D();
		const int32 StepCount = FMath::CeilToInt(Distance / 390.0f);
		for (int32 Step = 1; Step < StepCount; ++Step)
		{
			const float Alpha = static_cast<float>(Step) / StepCount;
			FVector WorldPoint = FMath::Lerp(UEGT1WorldLayout::GetSanctuaryLocation(), WaystoneLocation, Alpha);
			WorldPoint += FVector(Random.FRandRange(-36.0f, 36.0f), Random.FRandRange(-36.0f, 36.0f), 0.0f);
			if (!IsInsideTile(WorldPoint, 80.0f))
			{
				continue;
			}
			WorldPoint.Z = UEGT1WorldLayout::SampleRegion(WorldPoint).SurfaceHeight + 9.0f;
			const float Yaw = Direction.Rotation().Yaw + Random.FRandRange(-5.0f, 5.0f);
			TrailInstances->AddInstance(FTransform(FRotator(0.0f, Yaw, 0.0f), WorldPoint,
				FVector(Random.FRandRange(1.35f, 1.8f), Random.FRandRange(0.72f, 1.05f), 0.13f)), true);
		}
	}
}

void AUEGT1BiomeTile::GenerateVegetation(FRandomStream& Random)
{
	const float HalfTile = TileSize * 0.5f;
	const int32 TreeCandidates = FMath::CeilToInt(TreesPerTile * 1.75f);
	for (int32 Index = 0; Index < TreeCandidates; ++Index)
	{
		FVector WorldPosition = GetActorLocation() + FVector(Random.FRandRange(-HalfTile + 150.0f, HalfTile - 150.0f), Random.FRandRange(-HalfTile + 150.0f, HalfTile - 150.0f), 0.0f);
		const FUEGT1RegionSample Sample = UEGT1WorldLayout::SampleRegion(WorldPosition);
		const float Density = Sample.Biomes.Meadow * 0.82f + Sample.Biomes.Farmland * 0.15f + Sample.Biomes.Highlands * 0.58f +
			Sample.Biomes.Tropical * 1.65f + Sample.Biomes.Coast * 0.10f;
		if (Random.FRandRange(0.0f, 1.65f) > Density || Sample.WaterDepth > 10.0f || UEGT1WorldLayout::IsReservedGameplaySpace(WorldPosition, 170.0f))
		{
			continue;
		}

		WorldPosition.Z = Sample.SurfaceHeight;
		const float TropicalScale = 1.0f + Sample.Biomes.Tropical * 0.65f;
		const float HighlandScale = 1.0f + Sample.Biomes.Highlands * 0.35f;
		const float Height = Random.FRandRange(340.0f, 570.0f) * TropicalScale * HighlandScale;
		const float Width = Random.FRandRange(0.72f, 1.25f) * TropicalScale;
		const float Yaw = Random.FRandRange(0.0f, 360.0f);
		TrunkInstances->AddInstance(FTransform(FRotator(0.0f, Yaw, 0.0f), WorldPosition + FVector(0.0f, 0.0f, Height * 0.42f),
			FVector(0.24f * Width, 0.24f * Width, Height * 0.0084f)), true);
		if (Sample.Biomes.Tropical > 0.38f)
		{
			BroadleafInstances->AddInstance(FTransform(FRotator(0.0f, Yaw, 0.0f), WorldPosition + FVector(0.0f, 0.0f, Height * 0.88f),
				FVector(1.25f * Width, 1.05f * Width, 0.78f * Width)), true);
			BroadleafInstances->AddInstance(FTransform(FRotator(0.0f, Yaw + 40.0f, 0.0f), WorldPosition + FVector(0.0f, 0.0f, Height * 1.08f),
				FVector(0.86f * Width, 0.82f * Width, 0.72f * Width)), true);
		}
		else
		{
			ConiferInstances->AddInstance(FTransform(FRotator(0.0f, Yaw, 0.0f), WorldPosition + FVector(0.0f, 0.0f, Height * 0.92f),
				FVector(1.28f * Width, 1.28f * Width, Height * 0.009f)), true);
		}
	}

	const int32 RockCandidates = FMath::CeilToInt(RocksPerTile * 1.6f);
	for (int32 Index = 0; Index < RockCandidates; ++Index)
	{
		FVector WorldPosition = GetActorLocation() + FVector(Random.FRandRange(-HalfTile, HalfTile), Random.FRandRange(-HalfTile, HalfTile), 0.0f);
		const FUEGT1RegionSample Sample = UEGT1WorldLayout::SampleRegion(WorldPosition);
		const float Density = 0.18f + Sample.Biomes.Highlands * 1.25f + Sample.Biomes.Coast * 0.35f;
		if (Random.FRandRange(0.0f, 1.6f) > Density || Sample.WaterDepth > 260.0f || UEGT1WorldLayout::IsReservedGameplaySpace(WorldPosition, 90.0f))
		{
			continue;
		}
		WorldPosition.Z = Sample.SurfaceHeight + Random.FRandRange(18.0f, 48.0f);
		const float MountainScale = 1.0f + Sample.Biomes.Highlands * Random.FRandRange(0.4f, 1.8f);
		RockInstances->AddInstance(FTransform(FRotator(Random.FRandRange(-18.0f, 18.0f), Random.FRandRange(0.0f, 360.0f), Random.FRandRange(-18.0f, 18.0f)),
			WorldPosition, FVector(Random.FRandRange(0.4f, 1.15f) * MountainScale, Random.FRandRange(0.4f, 1.0f) * MountainScale,
				Random.FRandRange(0.35f, 0.9f) * MountainScale)), true);
	}

	const int32 GrassCandidates = FMath::CeilToInt(GrassClustersPerTile * 1.4f);
	for (int32 Index = 0; Index < GrassCandidates; ++Index)
	{
		FVector WorldPosition = GetActorLocation() + FVector(Random.FRandRange(-HalfTile, HalfTile), Random.FRandRange(-HalfTile, HalfTile), 0.0f);
		const FUEGT1RegionSample Sample = UEGT1WorldLayout::SampleRegion(WorldPosition);
		const float Density = Sample.Biomes.Meadow * 0.8f + Sample.Biomes.Farmland * 0.48f + Sample.Biomes.Highlands * 0.24f + Sample.Biomes.Tropical * 1.25f;
		if (Random.FRandRange(0.0f, 1.3f) > Density || Sample.WaterDepth > 5.0f || UEGT1WorldLayout::IsReservedGameplaySpace(WorldPosition, -80.0f))
		{
			continue;
		}
		WorldPosition.Z = Sample.SurfaceHeight + 28.0f;
		const float TropicalScale = 1.0f + Sample.Biomes.Tropical * 0.9f;
		GrassInstances->AddInstance(FTransform(FRotator(0.0f, Random.FRandRange(0.0f, 360.0f), 0.0f), WorldPosition,
			FVector(Random.FRandRange(0.12f, 0.24f) * TropicalScale, Random.FRandRange(0.12f, 0.24f) * TropicalScale,
				Random.FRandRange(0.34f, 0.72f) * TropicalScale)), true);
	}
}

void AUEGT1BiomeTile::GenerateRegionalFeatures(FRandomStream& Random)
{
	const FUEGT1RegionSample TileSample = UEGT1WorldLayout::SampleRegion(GetActorLocation());
	const float HalfTile = TileSize * 0.5f;
	if (TileSample.Biomes.Farmland > 0.24f && !UEGT1WorldLayout::IsPrimaryRoute(GetActorLocation(), HalfTile * 0.72f))
	{
		const bool bRowsAlongY = (TileCoordinate.X & 1) == 0;
		for (int32 Row = -7; Row <= 7; ++Row)
		{
			FVector Position = GetActorLocation();
			if (bRowsAlongY)
			{
				Position.X += Row * 155.0f;
			}
			else
			{
				Position.Y += Row * 155.0f;
			}
			const FUEGT1RegionSample Sample = UEGT1WorldLayout::SampleRegion(Position);
			Position.Z = Sample.SurfaceHeight + 12.0f;
			CropInstances->AddInstance(FTransform(FRotator(0.0f, bRowsAlongY ? 90.0f : 0.0f, 0.0f), Position,
				FVector(22.0f, 0.42f, 0.18f)), true);
		}
	}

	if (TileSample.Biomes.Highlands > 0.20f)
	{
		const int32 PeakCount = FMath::Clamp(FMath::RoundToInt(TileSample.Biomes.Highlands * 4.5f), 1, 5);
		for (int32 Index = 0; Index < PeakCount; ++Index)
		{
			FVector Position = GetActorLocation() + FVector(Random.FRandRange(-HalfTile, HalfTile), Random.FRandRange(-HalfTile, HalfTile), 0.0f);
			const FUEGT1RegionSample Sample = UEGT1WorldLayout::SampleRegion(Position);
			const float HeightScale = Random.FRandRange(7.0f, 18.0f) * (0.55f + Sample.Biomes.Highlands);
			Position.Z = Sample.SurfaceHeight + HeightScale * 48.0f;
			PeakInstances->AddInstance(FTransform(FRotator(0.0f, Random.FRandRange(0.0f, 360.0f), 0.0f), Position,
				FVector(Random.FRandRange(4.0f, 8.5f), Random.FRandRange(4.0f, 8.5f), HeightScale)), true);
		}
	}
}

FLinearColor AUEGT1BiomeTile::GetBlendedGroundColor(const FUEGT1BiomeWeights& Weights) const
{
	FLinearColor Color = UEGT1Palette::TownRoad * Weights.Town + UEGT1Palette::Meadow * Weights.Meadow +
		UEGT1Palette::Farmland * Weights.Farmland + UEGT1Palette::Highland * Weights.Highlands +
		UEGT1Palette::Tropical * Weights.Tropical + UEGT1Palette::Sand * Weights.Coast + UEGT1Palette::Seabed * Weights.Ocean;
	Color.A = 1.0f;
	return Color;
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
	int32 Count = 0;
	for (const UHierarchicalInstancedStaticMeshComponent* Component : { GroundInstances, TrailInstances, TrunkInstances, ConiferInstances,
		BroadleafInstances, RockInstances, GrassInstances, CropInstances, WaterInstances, WaveInstances, PeakInstances })
	{
		Count += Component->GetInstanceCount();
	}
	return Count;
}

void AUEGT1BiomeTile::ApplyGraphicsSettings()
{
	const UUEGT1GameUserSettings* Settings = UUEGT1GameUserSettings::Get();
	const bool bShowFoliage = !Settings || Settings->IsFeatureEnabled(EUEGT1GraphicsFeature::Foliage);
	for (UHierarchicalInstancedStaticMeshComponent* Component : { TrunkInstances, ConiferInstances, BroadleafInstances, GrassInstances, CropInstances })
	{
		Component->SetVisibility(bShowFoliage, true);
	}
}
