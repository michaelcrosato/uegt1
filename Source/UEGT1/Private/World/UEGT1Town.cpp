#include "World/UEGT1Town.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/CollisionProfile.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UEGT1LogChannels.h"
#include "UObject/ConstructorHelpers.h"
#include "World/UEGT1Palette.h"
#include "World/UEGT1VisualMaterials.h"
#include "World/UEGT1WorldLayout.h"

AUEGT1Town::AUEGT1Town()
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
	RoadInstances = CreateInstanceComponent(TEXT("RoadInstances"), CubeFinder.Object, false);
	WarmWallInstances = CreateInstanceComponent(TEXT("WarmWallInstances"), CubeFinder.Object, true);
	CoolWallInstances = CreateInstanceComponent(TEXT("CoolWallInstances"), CubeFinder.Object, true);
	RoofInstances = CreateInstanceComponent(TEXT("RoofInstances"), CubeFinder.Object, true);
	TrimInstances = CreateInstanceComponent(TEXT("TrimInstances"), CubeFinder.Object, false);
	PierInstances = CreateInstanceComponent(TEXT("PierInstances"), CubeFinder.Object, true);
	PostInstances = CreateInstanceComponent(TEXT("PostInstances"), CylinderFinder.Object, true);
	LampInstances = CreateInstanceComponent(TEXT("LampInstances"), SphereFinder.Object, false);
	LighthouseInstances = CreateInstanceComponent(TEXT("LighthouseInstances"), CylinderFinder.Object, true);
	LighthouseRoofInstances = CreateInstanceComponent(TEXT("LighthouseRoofInstances"), ConeFinder.Object, false);
}

UHierarchicalInstancedStaticMeshComponent* AUEGT1Town::CreateInstanceComponent(const FName Name, UStaticMesh* Mesh, bool bCollision)
{
	UHierarchicalInstancedStaticMeshComponent* Component = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(Name);
	Component->SetupAttachment(SceneRoot);
	Component->SetStaticMesh(Mesh);
	Component->SetMobility(EComponentMobility::Static);
	Component->SetCollisionEnabled(bCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	Component->SetCollisionProfileName(bCollision ? UCollisionProfile::BlockAll_ProfileName : UCollisionProfile::NoCollision_ProfileName);
	Component->SetCanEverAffectNavigation(false);
	Component->SetCullDistances(12000, 36000);
	return Component;
}

void AUEGT1Town::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RebuildTown();
}

void AUEGT1Town::BeginPlay()
{
	Super::BeginPlay();
	RebuildTown();
	UE_LOG(LogUEGT1, Display, TEXT("Town foundation ready: Buildings=%d Instances=%d Waterfront=Pier+Lighthouse"),
		BuildingCount, GetGeneratedInstanceCount());
}

void AUEGT1Town::RebuildTown()
{
	for (UHierarchicalInstancedStaticMeshComponent* Component : { RoadInstances, WarmWallInstances, CoolWallInstances, RoofInstances,
		TrimInstances, PierInstances, PostInstances, LampInstances, LighthouseInstances, LighthouseRoofInstances })
	{
		Component->ClearInstances();
	}
	BuildingCount = 0;

	FRandomStream Random(UEGT1WorldLayout::GetWorldSeed() + TownSeedOffset);
	AddRoadsAndPlaza();
	static const FVector BuildingSites[] = {
		FVector(-3200.0f, 2550.0f, 0.0f), FVector(-1600.0f, 2550.0f, 0.0f), FVector(0.0f, 2650.0f, 0.0f),
		FVector(1600.0f, 2550.0f, 0.0f), FVector(3200.0f, 2450.0f, 0.0f),
		FVector(-3200.0f, -2700.0f, 0.0f), FVector(-1600.0f, -2650.0f, 0.0f),
		FVector(1700.0f, -2700.0f, 0.0f), FVector(3250.0f, -2550.0f, 0.0f),
		FVector(-3400.0f, -1050.0f, 0.0f), FVector(3450.0f, -1050.0f, 0.0f), FVector(3500.0f, 850.0f, 0.0f)
	};
	for (const FVector& Site : BuildingSites)
	{
		if (!UEGT1WorldLayout::IsPrimaryRoute(Site, 430.0f))
		{
			AddBuilding(Random, Site);
		}
	}
	AddWaterfront();
	AddStreetFurniture();

	AssignColorMaterial(RoadInstances, UEGT1Palette::TownRoad, 0.92f, 0.18f, EUEGT1VisualMaterial::Surface);
	AssignColorMaterial(WarmWallInstances, UEGT1Palette::PlasterWarm, 0.78f, 0.28f, EUEGT1VisualMaterial::Surface);
	AssignColorMaterial(CoolWallInstances, UEGT1Palette::PlasterCool, 0.74f, 0.30f, EUEGT1VisualMaterial::Surface);
	AssignColorMaterial(RoofInstances, UEGT1Palette::RoofTeal, 0.30f, 0.62f, EUEGT1VisualMaterial::Surface, 0.04f);
	AssignColorMaterial(TrimInstances, UEGT1Palette::Amber * 0.82f, 0.52f, 0.46f, EUEGT1VisualMaterial::Surface, 0.03f);
	AssignColorMaterial(PierInstances, UEGT1Palette::Bark * 1.35f, 0.86f, 0.20f, EUEGT1VisualMaterial::Surface);
	AssignColorMaterial(PostInstances, UEGT1Palette::Bark * 0.72f, 0.88f, 0.18f, EUEGT1VisualMaterial::Surface);
	AssignColorMaterial(LampInstances, UEGT1Palette::Amber, 0.16f, 0.82f, EUEGT1VisualMaterial::Glow, 0.0f, 3.2f);
	AssignColorMaterial(LighthouseInstances, UEGT1Palette::Paper * 0.92f, 0.66f, 0.34f, EUEGT1VisualMaterial::Surface);
	AssignColorMaterial(LighthouseRoofInstances, UEGT1Palette::Signal * 0.72f, 0.24f, 0.70f, EUEGT1VisualMaterial::Surface, 0.06f);
}

void AUEGT1Town::AddBuilding(FRandomStream& Random, const FVector& Position)
{
	const float GroundHeight = UEGT1WorldLayout::SampleRegion(Position).SurfaceHeight;
	const float Width = Random.FRandRange(850.0f, 1250.0f);
	const float Depth = Random.FRandRange(720.0f, 980.0f);
	const float Height = Random.FRandRange(580.0f, 980.0f);
	const float FacingYaw = (-Position).Rotation().Yaw;
	const FRotator Facing(0.0f, FacingYaw, 0.0f);
	UHierarchicalInstancedStaticMeshComponent* Walls = (BuildingCount & 1) == 0 ? WarmWallInstances : CoolWallInstances;
	Walls->AddInstance(FTransform(Facing, Position + FVector(0.0f, 0.0f, GroundHeight + Height * 0.5f),
		FVector(Width / 100.0f, Depth / 100.0f, Height / 100.0f)), true);
	RoofInstances->AddInstance(FTransform(Facing, Position + FVector(0.0f, 0.0f, GroundHeight + Height + 45.0f),
		FVector(Width / 90.0f, Depth / 90.0f, 0.9f)), true);
	if ((BuildingCount % 3) == 0)
	{
		RoofInstances->AddInstance(FTransform(Facing, Position + FVector(0.0f, 0.0f, GroundHeight + Height + 120.0f),
			FVector(Width / 145.0f, Depth / 145.0f, 0.65f)), true);
	}

	const FVector Front = Facing.Vector();
	const FVector Right = FRotationMatrix(Facing).GetScaledAxis(EAxis::Y);
	const FVector FacadeCenter = Position + Front * (Width * 0.5f + 7.0f);
	TrimInstances->AddInstance(FTransform(Facing, FacadeCenter + FVector(0.0f, 0.0f, GroundHeight + 145.0f), FVector(0.16f, 1.25f, 2.8f)), true);
	for (const float Side : { -1.0f, 1.0f })
	{
		TrimInstances->AddInstance(FTransform(Facing, FacadeCenter + Right * Side * Depth * 0.24f + FVector(0.0f, 0.0f, GroundHeight + Height * 0.62f),
			FVector(0.14f, 0.72f, 0.85f)), true);
	}
	++BuildingCount;
}

void AUEGT1Town::AddRoadsAndPlaza()
{
	RoadInstances->AddInstance(FTransform(FRotator::ZeroRotator, FVector(0.0f, 0.0f, 5.0f), FVector(29.0f, 29.0f, 0.10f)), true);
	RoadInstances->AddInstance(FTransform(FRotator::ZeroRotator, FVector(0.0f, 0.0f, 4.0f), FVector(96.0f, 5.5f, 0.09f)), true);
	RoadInstances->AddInstance(FTransform(FRotator(0.0f, 90.0f, 0.0f), FVector(0.0f, 0.0f, 4.0f), FVector(96.0f, 5.5f, 0.09f)), true);
}

void AUEGT1Town::AddWaterfront()
{
	const float PierHeight = UEGT1WorldLayout::GetSeaLevel() + 55.0f;
	for (int32 Segment = 0; Segment < 11; ++Segment)
	{
		const float X = 4200.0f + Segment * 420.0f;
		PierInstances->AddInstance(FTransform(FRotator::ZeroRotator, FVector(X, 0.0f, PierHeight), FVector(4.5f, 4.2f, 0.24f)), true);
		for (const float Side : { -175.0f, 175.0f })
		{
			PostInstances->AddInstance(FTransform(FRotator::ZeroRotator, FVector(X, Side, PierHeight - 135.0f), FVector(0.18f, 0.18f, 3.2f)), true);
		}
	}

	const FVector LighthousePosition(7000.0f, -1650.0f, 0.0f);
	const float GroundHeight = UEGT1WorldLayout::SampleRegion(LighthousePosition).SurfaceHeight;
	LighthouseInstances->AddInstance(FTransform(FRotator::ZeroRotator,
		LighthousePosition + FVector(0.0f, 0.0f, GroundHeight + 620.0f), FVector(2.7f, 2.7f, 12.4f)), true);
	LighthouseRoofInstances->AddInstance(FTransform(FRotator::ZeroRotator,
		LighthousePosition + FVector(0.0f, 0.0f, GroundHeight + 1330.0f), FVector(3.6f, 3.6f, 3.4f)), true);
	LampInstances->AddInstance(FTransform(FRotator::ZeroRotator,
		LighthousePosition + FVector(0.0f, 0.0f, GroundHeight + 1220.0f), FVector(1.5f)), true);
}

void AUEGT1Town::AddStreetFurniture()
{
	for (const FVector2D Position : { FVector2D(-1250.0f, -1250.0f), FVector2D(1250.0f, -1250.0f),
		FVector2D(-1250.0f, 1250.0f), FVector2D(1250.0f, 1250.0f) })
	{
		PostInstances->AddInstance(FTransform(FRotator::ZeroRotator, FVector(Position.X, Position.Y, 180.0f), FVector(0.12f, 0.12f, 3.6f)), true);
		LampInstances->AddInstance(FTransform(FRotator::ZeroRotator, FVector(Position.X, Position.Y, 390.0f), FVector(0.42f)), true);
	}
}

void AUEGT1Town::AssignColorMaterial(UHierarchicalInstancedStaticMeshComponent* Component, const FLinearColor& Color,
	float Roughness, float Specular, EUEGT1VisualMaterial Style, float Metallic, float EmissiveStrength)
{
	UEGT1VisualMaterials::Apply(this, Component, ShapeMaterial, Style, Color, Roughness, Specular, Metallic, EmissiveStrength);
}

int32 AUEGT1Town::GetGeneratedInstanceCount() const
{
	int32 Count = 0;
	for (const UHierarchicalInstancedStaticMeshComponent* Component : { RoadInstances, WarmWallInstances, CoolWallInstances, RoofInstances,
		TrimInstances, PierInstances, PostInstances, LampInstances, LighthouseInstances, LighthouseRoofInstances })
	{
		Count += Component->GetInstanceCount();
	}
	return Count;
}
