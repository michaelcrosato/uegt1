#include "World/UEGT1Town.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/CollisionProfile.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Simulation/UEGT1TownDestinationComponent.h"
#include "Simulation/UEGT1TownActivityStation.h"
#include "Simulation/UEGT1TownSimulationSettings.h"
#include "Simulation/UEGT1TownSimulationSubsystem.h"
#include "UEGT1LogChannels.h"
#include "UObject/ConstructorHelpers.h"
#include "World/UEGT1Palette.h"
#include "World/UEGT1VisualMaterials.h"
#include "World/UEGT1TownGeneration.h"
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
	SidewalkInstances = CreateInstanceComponent(TEXT("SidewalkInstances"), CubeFinder.Object, false);
	TownGrassInstances = CreateInstanceComponent(TEXT("TownGrassInstances"), CubeFinder.Object, false);
	WarmWallInstances = CreateInstanceComponent(TEXT("WarmWallInstances"), CubeFinder.Object, true);
	CoolWallInstances = CreateInstanceComponent(TEXT("CoolWallInstances"), CubeFinder.Object, true);
	RoofInstances = CreateInstanceComponent(TEXT("RoofInstances"), CubeFinder.Object, true);
	TrimInstances = CreateInstanceComponent(TEXT("TrimInstances"), CubeFinder.Object, false);
	InteriorFloorInstances = CreateInstanceComponent(TEXT("InteriorFloorInstances"), CubeFinder.Object, false);
	InteriorWallInstances = CreateInstanceComponent(TEXT("InteriorWallInstances"), CubeFinder.Object, false);
	KitchenInstances = CreateInstanceComponent(TEXT("KitchenInstances"), CubeFinder.Object, false);
	BathroomInstances = CreateInstanceComponent(TEXT("BathroomInstances"), CylinderFinder.Object, false);
	ShowerInstances = CreateInstanceComponent(TEXT("ShowerInstances"), CubeFinder.Object, false);
	WorkstationInstances = CreateInstanceComponent(TEXT("WorkstationInstances"), CubeFinder.Object, false);
	BedFrameInstances = CreateInstanceComponent(TEXT("BedFrameInstances"), CubeFinder.Object, false);
	BeddingInstances = CreateInstanceComponent(TEXT("BeddingInstances"), CubeFinder.Object, false);
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
	RebuildTown(UEGT1WorldLayout::GetWorldSeed());
}

void AUEGT1Town::BeginPlay()
{
	Super::BeginPlay();
	int32 RuntimeSeed = UUEGT1TownSimulationSettings::Get().TownSeed;
	FParse::Value(FCommandLine::Get(), TEXT("UEGT1TownSeed="), RuntimeSeed);
	RebuildTown(RuntimeSeed);
	CreateDestinationComponents();
	if (UUEGT1TownSimulationSubsystem* Simulation = GetWorld()->GetSubsystem<UUEGT1TownSimulationSubsystem>())
	{
		Simulation->StartSimulation(RuntimeSeed);
	}
	CreateActivityStations();
	UE_LOG(LogUEGT1, Display, TEXT("Town foundation ready: Buildings=%d Interiors=%d ActivityStations=%d Instances=%d Venues=%d Streets=%d Beds=%d BoundsX=%.0f..%.0f Connected=%s Waterfront=Pier+Lighthouse"),
		BuildingCount, BuildingCount, ActivityStations.Num(), GetGeneratedInstanceCount(), GeneratedLayout.Lots.Num(), GeneratedLayout.Streets.Num(), GetBedCount(),
		GeneratedLayout.BoundsMin.X, GeneratedLayout.BoundsMax.X,
		UEGT1TownGeneration::IsFullyConnected(GeneratedLayout) ? TEXT("true") : TEXT("false"));
}

void AUEGT1Town::RegenerateFromSimulationSeed(int32 Seed)
{
	RebuildTown(Seed);
	CreateDestinationComponents();
	CreateActivityStations();
}

void AUEGT1Town::RebuildTown(int32 Seed)
{
	for (UTextRenderComponent* Sign : BuildingSigns)
	{
		if (Sign)
		{
			Sign->DestroyComponent();
		}
	}
	BuildingSigns.Reset();
	for (UPointLightComponent* Light : InteriorLights)
	{
		if (Light)
		{
			Light->DestroyComponent();
		}
	}
	InteriorLights.Reset();
	for (UHierarchicalInstancedStaticMeshComponent* Component : { RoadInstances, SidewalkInstances, TownGrassInstances,
		WarmWallInstances, CoolWallInstances, RoofInstances,
		TrimInstances, InteriorFloorInstances, InteriorWallInstances, KitchenInstances, BathroomInstances, ShowerInstances,
		WorkstationInstances, BedFrameInstances, BeddingInstances, PierInstances, PostInstances, LampInstances, LighthouseInstances, LighthouseRoofInstances })
	{
		Component->ClearInstances();
	}
	BuildingCount = 0;
	GeneratedLayout = UEGT1TownGeneration::Generate(Seed, UUEGT1TownSimulationSettings::Get().MakeTuning());
	AddRoadsAndPlaza(GeneratedLayout);
	for (const FUEGT1GeneratedTownLot& Lot : GeneratedLayout.Lots)
	{
		const float GroundHeight = UEGT1WorldLayout::SampleRegion(Lot.Center).SurfaceHeight;
		TownGrassInstances->AddInstance(FTransform(FRotator::ZeroRotator,
			Lot.Center + FVector(0.0f, 0.0f, GroundHeight + 1.0f),
			FVector(Lot.Footprint.X / 72.0f, Lot.Footprint.Y / 72.0f, 0.035f)), true);
		if (Lot.VenueType != EUEGT1TownVenueType::Park)
		{
			AddBuilding(Lot, BuildingCount);
		}
		AddBeds(Lot);
	}
	AddWaterfront();
	AddStreetFurniture();

	AssignColorMaterial(RoadInstances, UEGT1Palette::TownRoad, 0.92f, 0.18f, EUEGT1VisualMaterial::Surface);
	AssignColorMaterial(SidewalkInstances, UEGT1Palette::Paper * 0.66f, 0.88f, 0.20f, EUEGT1VisualMaterial::Surface);
	AssignColorMaterial(TownGrassInstances, UEGT1Palette::Meadow * 0.72f, 0.96f, 0.08f, EUEGT1VisualMaterial::Surface);
	AssignColorMaterial(WarmWallInstances, UEGT1Palette::PlasterWarm, 0.78f, 0.28f, EUEGT1VisualMaterial::Surface);
	AssignColorMaterial(CoolWallInstances, UEGT1Palette::PlasterCool, 0.74f, 0.30f, EUEGT1VisualMaterial::Surface);
	AssignColorMaterial(RoofInstances, UEGT1Palette::RoofTeal, 0.30f, 0.62f, EUEGT1VisualMaterial::Surface, 0.04f);
	AssignColorMaterial(TrimInstances, UEGT1Palette::Amber * 0.82f, 0.52f, 0.46f, EUEGT1VisualMaterial::Surface, 0.03f);
	AssignColorMaterial(InteriorFloorInstances, UEGT1Palette::Bark * 1.25f, 0.82f, 0.20f, EUEGT1VisualMaterial::Glow, 0.0f, 2.0f);
	AssignColorMaterial(InteriorWallInstances, UEGT1Palette::Paper * 0.76f, 0.88f, 0.18f, EUEGT1VisualMaterial::Glow, 0.0f, 2.0f);
	AssignColorMaterial(KitchenInstances, UEGT1Palette::RoofTeal * 0.72f, 0.46f, 0.42f, EUEGT1VisualMaterial::Glow, 0.05f, 10.0f);
	AssignColorMaterial(BathroomInstances, UEGT1Palette::Paper * 0.94f, 0.28f, 0.66f, EUEGT1VisualMaterial::Glow, 0.0f, 8.0f);
	AssignColorMaterial(ShowerInstances, UEGT1Palette::Signal * 0.72f, 0.24f, 0.68f, EUEGT1VisualMaterial::Glow, 0.03f, 12.0f);
	AssignColorMaterial(WorkstationInstances, UEGT1Palette::Amber * 0.62f, 0.66f, 0.32f, EUEGT1VisualMaterial::Glow, 0.0f, 8.0f);
	AssignColorMaterial(BedFrameInstances, UEGT1Palette::Bark * 0.82f, 0.88f, 0.18f, EUEGT1VisualMaterial::Glow, 0.0f, 3.0f);
	AssignColorMaterial(BeddingInstances, UEGT1Palette::Paper * 0.92f, 0.82f, 0.24f, EUEGT1VisualMaterial::Glow, 0.0f, 8.0f);
	AssignColorMaterial(PierInstances, UEGT1Palette::Bark * 1.35f, 0.86f, 0.20f, EUEGT1VisualMaterial::Surface);
	AssignColorMaterial(PostInstances, UEGT1Palette::Bark * 0.72f, 0.88f, 0.18f, EUEGT1VisualMaterial::Surface);
	AssignColorMaterial(LampInstances, UEGT1Palette::Amber, 0.16f, 0.82f, EUEGT1VisualMaterial::Glow, 0.0f, 3.2f);
	AssignColorMaterial(LighthouseInstances, UEGT1Palette::Paper * 0.92f, 0.66f, 0.34f, EUEGT1VisualMaterial::Surface);
	AssignColorMaterial(LighthouseRoofInstances, UEGT1Palette::Signal * 0.72f, 0.24f, 0.70f, EUEGT1VisualMaterial::Surface, 0.06f);
}

void AUEGT1Town::CreateDestinationComponents()
{
	for (UUEGT1TownDestinationComponent* Component : DestinationComponents)
	{
		if (Component)
		{
			Component->DestroyComponent();
		}
	}
	DestinationComponents.Reset();
	if (GeneratedLayout.Lots.IsEmpty())
	{
		return;
	}
	for (const FUEGT1GeneratedTownLot& Lot : GeneratedLayout.Lots)
	{
		const FName ComponentName(*FString::Printf(TEXT("Destination_%s"), *Lot.LotId.ToString()));
		UUEGT1TownDestinationComponent* Component = NewObject<UUEGT1TownDestinationComponent>(this, ComponentName);
		Component->SetupAttachment(SceneRoot);
		Component->ConfigureFromGeneratedLot(Lot);
		AddInstanceComponent(Component);
		Component->RegisterComponent();
		DestinationComponents.Add(Component);
	}
}

void AUEGT1Town::CreateActivityStations()
{
	for (AUEGT1TownActivityStation* Station : ActivityStations)
	{
		if (Station)
		{
			Station->Destroy();
		}
	}
	ActivityStations.Reset();
	auto AddStation = [&](const FUEGT1GeneratedTownLot& Lot, EUEGT1SimActionType Action, const FVector& Location)
	{
		AUEGT1TownActivityStation* Station = GetWorld()->SpawnActor<AUEGT1TownActivityStation>(
			AUEGT1TownActivityStation::StaticClass(), Location, Lot.FacingRotation);
		if (Station)
		{
			Station->Configure(Lot.LotId, Action, Lot.DisplayName, Lot.JobTitle, Lot.HourlyRate);
			ActivityStations.Add(Station);
		}
	};
	for (const FUEGT1GeneratedTownLot& Lot : GeneratedLayout.Lots)
	{
		switch (Lot.VenueType)
		{
		case EUEGT1TownVenueType::Home:
			if (!Lot.Beds.IsEmpty())
			{
				AddStation(Lot, EUEGT1SimActionType::Sleep, Lot.Beds[0].WorldLocation);
			}
			AddStation(Lot, EUEGT1SimActionType::Eat, Lot.KitchenLocation);
			AddStation(Lot, EUEGT1SimActionType::Hygiene, Lot.ShowerLocation);
			break;
		case EUEGT1TownVenueType::FoodVenue:
			AddStation(Lot, EUEGT1SimActionType::Eat, Lot.ActivityLocation);
			break;
		case EUEGT1TownVenueType::Workplace:
			AddStation(Lot, EUEGT1SimActionType::Work, Lot.ActivityLocation);
			break;
		case EUEGT1TownVenueType::SocialVenue:
		case EUEGT1TownVenueType::Park:
			AddStation(Lot, EUEGT1SimActionType::Socialize, Lot.ActivityLocation);
			break;
		default:
			break;
		}
	}
}

void AUEGT1Town::AddBuilding(const FUEGT1GeneratedTownLot& Lot, int32 BuildingIndex)
{
	const FVector Position = Lot.Center;
	const float GroundHeight = UEGT1WorldLayout::SampleRegion(Position).SurfaceHeight;
	const float Width = Lot.Footprint.X;
	const float Depth = Lot.Footprint.Y;
	const float Height = Lot.BuildingHeight;
	const FRotator Facing = Lot.FacingRotation;
	const FVector Front = Facing.Vector();
	const FVector Right = FRotationMatrix(Facing).GetScaledAxis(EAxis::Y);
	UHierarchicalInstancedStaticMeshComponent* Walls = (BuildingIndex & 1) == 0 ? WarmWallInstances : CoolWallInstances;
	auto AddLocalBox = [&](UHierarchicalInstancedStaticMeshComponent* Component, float LocalX, float LocalY, float Z,
		float SizeX, float SizeY, float SizeZ)
	{
		Component->AddInstance(FTransform(Facing, Position + Front * LocalX + Right * LocalY + FVector(0.0f, 0.0f, Z),
			FVector(SizeX / 100.0f, SizeY / 100.0f, SizeZ / 100.0f)), true);
	};
	constexpr float WallThickness = 24.0f;
	constexpr float DoorWidth = 210.0f;
	constexpr float DoorHeight = 270.0f;
	const float FrontSideWidth = FMath::Max(60.0f, (Depth - DoorWidth) * 0.5f);
	AddLocalBox(Walls, Width * 0.5f, -(DoorWidth + FrontSideWidth) * 0.5f, GroundHeight + Height * 0.5f,
		WallThickness, FrontSideWidth, Height);
	AddLocalBox(Walls, Width * 0.5f, (DoorWidth + FrontSideWidth) * 0.5f, GroundHeight + Height * 0.5f,
		WallThickness, FrontSideWidth, Height);
	AddLocalBox(Walls, Width * 0.5f, 0.0f, GroundHeight + DoorHeight + (Height - DoorHeight) * 0.5f,
		WallThickness, DoorWidth, FMath::Max(20.0f, Height - DoorHeight));
	AddLocalBox(Walls, -Width * 0.5f, 0.0f, GroundHeight + Height * 0.5f, WallThickness, Depth, Height);
	AddLocalBox(Walls, 0.0f, -Depth * 0.5f, GroundHeight + Height * 0.5f, Width, WallThickness, Height);
	AddLocalBox(Walls, 0.0f, Depth * 0.5f, GroundHeight + Height * 0.5f, Width, WallThickness, Height);
	AddLocalBox(InteriorFloorInstances, 0.0f, 0.0f, GroundHeight + 5.0f, Width - 30.0f, Depth - 30.0f, 10.0f);
	const float SkylightWidth = Width * 0.74f;
	const float SkylightDepth = Depth * 0.70f;
	const float RoofSideWidth = (Width - SkylightWidth) * 0.5f;
	const float RoofSideDepth = (Depth - SkylightDepth) * 0.5f;
	const float RoofZ = GroundHeight + Height + 45.0f;
	for (const float Side : { -1.0f, 1.0f })
	{
		AddLocalBox(RoofInstances, Side * (SkylightWidth + RoofSideWidth) * 0.5f, 0.0f, RoofZ,
			RoofSideWidth + 24.0f, Depth + 48.0f, 90.0f);
		AddLocalBox(RoofInstances, 0.0f, Side * (SkylightDepth + RoofSideDepth) * 0.5f, RoofZ,
			SkylightWidth + 24.0f, RoofSideDepth + 24.0f, 90.0f);
	}

	const FVector FacadeCenter = Position + Front * (Width * 0.5f + 7.0f);
	TrimInstances->AddInstance(FTransform(Facing, FacadeCenter + FVector(0.0f, 0.0f, GroundHeight + 145.0f), FVector(0.16f, 1.25f, 2.8f)), true);
	for (const float Side : { -1.0f, 1.0f })
	{
		TrimInstances->AddInstance(FTransform(Facing, FacadeCenter + Right * Side * Depth * 0.24f + FVector(0.0f, 0.0f, GroundHeight + Height * 0.62f),
			FVector(0.14f, 0.72f, 0.85f)), true);
	}

	// Open-plan interior zones remain easy to navigate while visibly providing every required amenity.
	AddLocalBox(InteriorWallInstances, -Width * 0.12f, Depth * 0.12f, GroundHeight + 95.0f,
		18.0f, Depth * 0.42f, 190.0f);
	KitchenInstances->AddInstance(FTransform(Facing, Lot.KitchenLocation + FVector(0.0f, 0.0f, 48.0f),
		FVector(1.65f, 0.55f, 0.90f)), true);
	KitchenInstances->AddInstance(FTransform(Facing, Lot.KitchenLocation - Right * 92.0f + FVector(0.0f, 0.0f, 105.0f),
		FVector(0.62f, 0.58f, 2.05f)), true);
	BathroomInstances->AddInstance(FTransform(Facing, Lot.BathroomLocation + FVector(0.0f, 0.0f, 42.0f),
		FVector(0.50f, 0.50f, 0.84f)), true);
	ShowerInstances->AddInstance(FTransform(Facing, Lot.ShowerLocation + FVector(0.0f, 0.0f, 8.0f),
		FVector(1.15f, 1.15f, 0.16f)), true);
	for (const float Side : { -1.0f, 1.0f })
	{
		ShowerInstances->AddInstance(FTransform(Facing, Lot.ShowerLocation + Right * Side * 58.0f + FVector(0.0f, 0.0f, 105.0f),
			FVector(0.10f, 0.10f, 2.10f)), true);
	}
	if (Lot.VenueType == EUEGT1TownVenueType::Workplace || Lot.VenueType == EUEGT1TownVenueType::FoodVenue ||
		Lot.VenueType == EUEGT1TownVenueType::SocialVenue)
	{
		WorkstationInstances->AddInstance(FTransform(Facing, Lot.ActivityLocation + FVector(0.0f, 0.0f, 46.0f),
			FVector(1.55f, 0.72f, 0.86f)), true);
		const FString Type = Lot.BusinessType.ToLower();
		if (Type.Contains(TEXT("grocery")) || Type.Contains(TEXT("retail")))
		{
			for (const float Side : { -1.0f, 1.0f })
			{
				WorkstationInstances->AddInstance(FTransform(Facing, Lot.ActivityLocation + Right * Side * 165.0f + FVector(0.0f, 0.0f, 92.0f),
					FVector(2.8f, 0.42f, 1.84f)), true);
			}
		}
		else if (Type.Contains(TEXT("restaurant")) || Type.Contains(TEXT("cafe")) || Type.Contains(TEXT("bakery")))
		{
			KitchenInstances->AddInstance(FTransform(Facing, Lot.ActivityLocation - Front * 145.0f + FVector(0.0f, 0.0f, 48.0f),
				FVector(2.4f, 0.62f, 0.90f)), true);
		}
		else if (Type.Contains(TEXT("factory")) || Type.Contains(TEXT("repair")) || Type.Contains(TEXT("logistics")) ||
			Type.Contains(TEXT("transport")))
		{
			for (const float Offset : { -190.0f, 0.0f, 190.0f })
			{
				WorkstationInstances->AddInstance(FTransform(Facing, Lot.ActivityLocation + Right * Offset - Front * 135.0f + FVector(0.0f, 0.0f, 58.0f),
					FVector(1.15f, 0.82f, 1.12f)), true);
			}
		}
		else
		{
			for (const float Side : { -1.0f, 1.0f })
			{
				WorkstationInstances->AddInstance(FTransform(Facing, Lot.ActivityLocation + Right * Side * 145.0f - Front * 105.0f + FVector(0.0f, 0.0f, 39.0f),
					FVector(1.25f, 0.72f, 0.76f)), true);
			}
		}
	}

	LampInstances->AddInstance(FTransform(Facing, Position + FVector(0.0f, 0.0f, GroundHeight + Height * 0.68f),
		FVector(0.24f)), true);
	if (GetWorld() && GetWorld()->IsGameWorld())
	{
		UTextRenderComponent* Sign = NewObject<UTextRenderComponent>(this, NAME_None, RF_Transient);
		Sign->SetupAttachment(SceneRoot);
		Sign->SetMobility(EComponentMobility::Static);
		Sign->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
		Sign->SetTextRenderColor(FColor(255, 188, 92));
		Sign->SetWorldSize(44.0f);
		Sign->SetText(FText::AsCultureInvariant(Lot.JobTitle.IsEmpty() ? Lot.DisplayName :
			FString::Printf(TEXT("%s\n%s"), *Lot.DisplayName, *Lot.BusinessType)));
		Sign->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		AddInstanceComponent(Sign);
		Sign->RegisterComponent();
		Sign->SetWorldLocation(FacadeCenter + Front * 18.0f + FVector(0.0f, 0.0f, GroundHeight + Height * 0.48f));
		Sign->SetWorldRotation(Facing);
		BuildingSigns.Add(Sign);

		UPointLightComponent* InteriorLight = NewObject<UPointLightComponent>(this, NAME_None, RF_Transient);
		InteriorLight->SetupAttachment(SceneRoot);
		InteriorLight->SetMobility(EComponentMobility::Movable);
		InteriorLight->SetLightColor(FLinearColor(1.0f, 0.72f, 0.46f));
		InteriorLight->SetIntensity(30000.0f);
		InteriorLight->SetAttenuationRadius(FMath::Max(Width, Depth) * 0.82f);
		InteriorLight->SetCastShadows(false);
		InteriorLight->SetVolumetricScatteringIntensity(0.15f);
		InteriorLight->SetVisibility(true);
		AddInstanceComponent(InteriorLight);
		InteriorLight->RegisterComponent();
		InteriorLight->SetWorldLocation(Position + FVector(0.0f, 0.0f, GroundHeight + Height * 0.52f));
		InteriorLights.Add(InteriorLight);
	}
	++BuildingCount;
}

void AUEGT1Town::AddBeds(const FUEGT1GeneratedTownLot& Lot)
{
	for (const FUEGT1TownBedState& Bed : Lot.Beds)
	{
		BedFrameInstances->AddInstance(FTransform(Bed.Rotation, Bed.WorldLocation + FVector(0.0f, 0.0f, 18.0f),
			FVector(1.95f, 0.92f, 0.22f)), true);
		BeddingInstances->AddInstance(FTransform(Bed.Rotation, Bed.WorldLocation + FVector(0.0f, 0.0f, 39.0f),
			FVector(1.72f, 0.78f, 0.20f)), true);
	}
	for (const FVector& BedLocation : Lot.AmenityBedLocations)
	{
		BedFrameInstances->AddInstance(FTransform(Lot.FacingRotation, BedLocation + FVector(0.0f, 0.0f, 18.0f),
			FVector(1.95f, 0.92f, 0.22f)), true);
		BeddingInstances->AddInstance(FTransform(Lot.FacingRotation, BedLocation + FVector(0.0f, 0.0f, 39.0f),
			FVector(1.72f, 0.78f, 0.20f)), true);
	}
}

void AUEGT1Town::AddRoadsAndPlaza(const FUEGT1GeneratedTownLayout& Layout)
{
	RoadInstances->AddInstance(FTransform(FRotator::ZeroRotator, FVector(0.0f, 0.0f, 5.0f), FVector(29.0f, 29.0f, 0.10f)), true);
	for (const FUEGT1GeneratedStreet& Street : Layout.Streets)
	{
		const FVector Delta = Street.End - Street.Start;
		const FVector Center = (Street.Start + Street.End) * 0.5f;
		const float Length = Delta.Size2D();
		const FRotator Rotation = Delta.Rotation();
		RoadInstances->AddInstance(FTransform(Rotation, Center, FVector(Length / 100.0f, Street.Width / 100.0f, 0.09f)), true);
		const FVector Side = FRotationMatrix(Rotation).GetScaledAxis(EAxis::Y);
		for (const float Sign : { -1.0f, 1.0f })
		{
			SidewalkInstances->AddInstance(FTransform(Rotation, Center + Side * Sign * (Street.Width * 0.5f + 95.0f) + FVector(0.0f, 0.0f, 5.0f),
				FVector(Length / 100.0f, 1.8f, 0.12f)), true);
		}
	}
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
	for (const UHierarchicalInstancedStaticMeshComponent* Component : { RoadInstances, SidewalkInstances, TownGrassInstances,
		WarmWallInstances, CoolWallInstances, RoofInstances,
		TrimInstances, InteriorFloorInstances, InteriorWallInstances, KitchenInstances, BathroomInstances, ShowerInstances,
		WorkstationInstances, BedFrameInstances, BeddingInstances, PierInstances, PostInstances, LampInstances, LighthouseInstances, LighthouseRoofInstances })
	{
		Count += Component->GetInstanceCount();
	}
	return Count;
}

int32 AUEGT1Town::GetBedCount() const
{
	int32 ResidentBeds = 0;
	for (const FUEGT1GeneratedTownLot& Lot : GeneratedLayout.Lots)
	{
		ResidentBeds += Lot.VenueType == EUEGT1TownVenueType::Home ? Lot.Beds.Num() : 0;
	}
	return ResidentBeds;
}
