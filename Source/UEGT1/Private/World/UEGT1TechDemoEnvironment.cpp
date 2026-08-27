#include "World/UEGT1TechDemoEnvironment.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/CollisionProfile.h"
#include "Materials/MaterialInterface.h"
#include "ProceduralMeshComponent.h"
#include "Settings/UEGT1GameUserSettings.h"
#include "UObject/ConstructorHelpers.h"
#include "World/UEGT1VisualMaterials.h"
#include "UEGT1LogChannels.h"

namespace
{
	constexpr float LakeWaterHeight = 145.0f;
	const FVector2D LakeCenter(0.0f, 3200.0f);

	float SmoothRange(float Min, float Max, float Value)
	{
		const float Alpha = FMath::Clamp((Value - Min) / FMath::Max(Max - Min, 1.0f), 0.0f, 1.0f);
		return Alpha * Alpha * (3.0f - 2.0f * Alpha);
	}

	float TrailCenterX(float Y)
	{
		return FMath::Sin((Y + 2200.0f) * 0.00042f) * 720.0f + FMath::Sin(Y * 0.0011f) * 180.0f;
	}
}

AUEGT1TechDemoEnvironment::AUEGT1TechDemoEnvironment()
{
	PrimaryActorTick.bCanEverTick = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeFinder(TEXT("/Engine/BasicShapes/Cone.Cone"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> TreeFinder(TEXT("/Game/ArchVis/SampleScene/Tree/HillTree_02.HillTree_02"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> RockFinder(TEXT("/Game/SimSandbox/Meshes/Rock.Rock"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	FallbackMaterial = MaterialFinder.Object;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SceneRoot->SetMobility(EComponentMobility::Static);
	SetRootComponent(SceneRoot);

	TerrainMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("TerrainMesh"));
	TerrainMesh->SetupAttachment(SceneRoot);
	TerrainMesh->SetMobility(EComponentMobility::Static);
	TerrainMesh->bUseAsyncCooking = true;
	TerrainMesh->bUseComplexAsSimpleCollision = true;
	TerrainMesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	TerrainMesh->SetCanEverAffectNavigation(true);

	LakeSurface = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("LakeSurface"));
	LakeSurface->SetupAttachment(SceneRoot);
	LakeSurface->SetMobility(EComponentMobility::Static);
	LakeSurface->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LakeSurface->SetGenerateOverlapEvents(false);
	LakeSurface->SetCanEverAffectNavigation(false);
	WaterInstances = CreateInstances(TEXT("WaterInstances"), CubeFinder.Object, false, 0, 30000);
	PathInstances = CreateInstances(TEXT("PathInstances"), CubeFinder.Object, true, 0, 22000);
	TreeInstances = CreateInstances(TEXT("TreeInstances"), TreeFinder.Object, false, 5000, 36000);
	TrunkInstances = CreateInstances(TEXT("TrunkInstances"), CylinderFinder.Object, true, 0, 30000);
	CanopyInstances = CreateInstances(TEXT("CanopyInstances"), SphereFinder.Object, false, 5000, 30000);
	PineInstances = CreateInstances(TEXT("PineInstances"), ConeFinder.Object, false, 5000, 32000);
	RockInstances = CreateInstances(TEXT("RockInstances"), RockFinder.Succeeded() ? RockFinder.Object : SphereFinder.Object, true, 1500, 26000);
	CliffInstances = CreateInstances(TEXT("CliffInstances"), RockFinder.Succeeded() ? RockFinder.Object : SphereFinder.Object, true, 0, 36000);
	GrassInstances = CreateInstances(TEXT("GrassInstances"), ConeFinder.Object, false, 800, 9500);
	FernInstances = CreateInstances(TEXT("FernInstances"), ConeFinder.Object, false, 1200, 12500);
	FlowerInstances = CreateInstances(TEXT("FlowerInstances"), SphereFinder.Object, false, 500, 8500);
	FallenLogInstances = CreateInstances(TEXT("FallenLogInstances"), CylinderFinder.Object, true, 1000, 18000);
	FoamInstances = CreateInstances(TEXT("FoamInstances"), SphereFinder.Object, false, 500, 16000);
	GrassInstances->SetCastShadow(false);
	FlowerInstances->SetCastShadow(false);
}

UHierarchicalInstancedStaticMeshComponent* AUEGT1TechDemoEnvironment::CreateInstances(
	FName Name, UStaticMesh* Mesh, bool bCollision, int32 StartCullDistance, int32 EndCullDistance)
{
	UHierarchicalInstancedStaticMeshComponent* Component = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(Name);
	Component->SetupAttachment(SceneRoot);
	Component->SetStaticMesh(Mesh);
	Component->SetMobility(EComponentMobility::Static);
	Component->SetCollisionEnabled(bCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	Component->SetCollisionProfileName(bCollision ? UCollisionProfile::BlockAll_ProfileName : UCollisionProfile::NoCollision_ProfileName);
	Component->SetGenerateOverlapEvents(false);
	Component->SetCanEverAffectNavigation(false);
	Component->SetCullDistances(StartCullDistance, EndCullDistance);
	return Component;
}

void AUEGT1TechDemoEnvironment::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RebuildEnvironment();
}

void AUEGT1TechDemoEnvironment::BeginPlay()
{
	Super::BeginPlay();
	RebuildEnvironment();
	UUEGT1GameUserSettings::OnGraphicsSettingsApplied.AddUObject(this, &AUEGT1TechDemoEnvironment::ApplyGraphicsSettings);
	ApplyGraphicsSettings();
	UE_LOG(LogUEGT1, Display, TEXT("Lumen Wilds showcase ready: TerrainVertices=%d Instances=%d Trees=%d Lumen=true VSM=true"),
		TerrainResolution * TerrainResolution, GetGeneratedInstanceCount(), GeneratedTreeCount);
}

void AUEGT1TechDemoEnvironment::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UUEGT1GameUserSettings::OnGraphicsSettingsApplied.RemoveAll(this);
	Super::EndPlay(EndPlayReason);
}

float AUEGT1TechDemoEnvironment::SampleTerrainHeight(const FVector2D& Position)
{
	const float Radius = Position.Size();
	const float Macro = FMath::PerlinNoise2D(Position * 0.000105f) * 1450.0f;
	const float Mid = FMath::PerlinNoise2D(Position * 0.00031f + FVector2D(17.4f, -9.7f)) * 520.0f;
	const float Detail = FMath::PerlinNoise2D(Position * 0.00092f + FVector2D(-31.2f, 22.8f)) * 130.0f;
	const float RidgeNoise = 1.0f - FMath::Abs(FMath::PerlinNoise2D(Position * 0.00016f + FVector2D(4.1f, 19.6f)));
	const float EdgeUplift = FMath::Square(SmoothRange(7200.0f, 17800.0f, Radius)) * 3600.0f;
	float Height = Macro + Mid + Detail + EdgeUplift + FMath::Pow(RidgeNoise, 3.0f) * SmoothRange(8500.0f, 17000.0f, Radius) * 1250.0f;

	const float LakeDistance = FVector2D::Distance(Position, LakeCenter);
	const float LakeBlend = 1.0f - SmoothRange(2700.0f, 4300.0f, LakeDistance);
	Height = FMath::Lerp(Height, LakeWaterHeight - 620.0f + LakeDistance * 0.035f, LakeBlend);

	if (Position.Y < 1800.0f)
	{
		const float TrailDistance = FMath::Abs(Position.X - TrailCenterX(Position.Y));
		const float TrailBlend = 1.0f - SmoothRange(260.0f, 1050.0f, TrailDistance);
		const float TrailHeight = FMath::PerlinNoise1D(Position.Y * 0.00015f) * 180.0f - 40.0f;
		Height = FMath::Lerp(Height, TrailHeight, TrailBlend * 0.72f);
	}
	return Height;
}

FVector AUEGT1TechDemoEnvironment::GetRecommendedPlayerStart()
{
	const FVector2D Position(0.0f, -13200.0f);
	return FVector(Position.X, Position.Y, SampleTerrainHeight(Position) + 180.0f);
}

float AUEGT1TechDemoEnvironment::SampleSlope(const FVector2D& Position) const
{
	constexpr float Offset = 160.0f;
	const float DeltaX = SampleTerrainHeight(Position + FVector2D(Offset, 0.0f)) - SampleTerrainHeight(Position - FVector2D(Offset, 0.0f));
	const float DeltaY = SampleTerrainHeight(Position + FVector2D(0.0f, Offset)) - SampleTerrainHeight(Position - FVector2D(0.0f, Offset));
	return FVector(-DeltaX / (Offset * 2.0f), -DeltaY / (Offset * 2.0f), 1.0f).GetSafeNormal().Z;
}

bool AUEGT1TechDemoEnvironment::IsLake(const FVector2D& Position, float Margin) const
{
	return FVector2D::Distance(Position, LakeCenter) < 3850.0f + Margin;
}

bool AUEGT1TechDemoEnvironment::IsTrail(const FVector2D& Position, float Margin) const
{
	return Position.Y < 2200.0f && FMath::Abs(Position.X - TrailCenterX(Position.Y)) < 470.0f + Margin;
}

void AUEGT1TechDemoEnvironment::RebuildEnvironment()
{
	TerrainResolution = FMath::Clamp(TerrainResolution, 65, 257);
	TerrainMesh->ClearAllMeshSections();
	LakeSurface->ClearAllMeshSections();
	for (UHierarchicalInstancedStaticMeshComponent* Component : { WaterInstances, PathInstances, TreeInstances, TrunkInstances, CanopyInstances,
		PineInstances, RockInstances, CliffInstances, GrassInstances, FernInstances, FlowerInstances, FallenLogInstances, FoamInstances })
	{
		Component->ClearInstances();
	}
	GeneratedTreeCount = 0;

	FRandomStream Random(584021);
	GenerateTerrain();
	GenerateWaterAndPaths(Random);
	GenerateForest(Random);
	GenerateGroundDetail(Random);
	GenerateRockFormations(Random);
	ApplyMaterials();
}

void AUEGT1TechDemoEnvironment::GenerateTerrain()
{
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FLinearColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;
	const int32 Resolution = TerrainResolution;
	const float HalfSize = TerrainSize * 0.5f;
	const float Step = TerrainSize / static_cast<float>(Resolution - 1);
	Vertices.Reserve(Resolution * Resolution);
	Normals.Reserve(Resolution * Resolution);
	UVs.Reserve(Resolution * Resolution);
	VertexColors.Reserve(Resolution * Resolution);
	Tangents.Reserve(Resolution * Resolution);
	Triangles.Reserve((Resolution - 1) * (Resolution - 1) * 6);

	for (int32 Y = 0; Y < Resolution; ++Y)
	{
		for (int32 X = 0; X < Resolution; ++X)
		{
			const FVector2D Position(-HalfSize + X * Step, -HalfSize + Y * Step);
			const float Height = SampleTerrainHeight(Position);
			const float HeightLeft = SampleTerrainHeight(Position - FVector2D(Step, 0.0f));
			const float HeightRight = SampleTerrainHeight(Position + FVector2D(Step, 0.0f));
			const float HeightBack = SampleTerrainHeight(Position - FVector2D(0.0f, Step));
			const float HeightFront = SampleTerrainHeight(Position + FVector2D(0.0f, Step));
			const FVector Normal = FVector(-(HeightRight - HeightLeft) / (Step * 2.0f), -(HeightFront - HeightBack) / (Step * 2.0f), 1.0f).GetSafeNormal();
			const float RockBlend = FMath::Pow(1.0f - Normal.Z, 1.35f);
			const float HeightBlend = SmoothRange(1600.0f, 4300.0f, Height);
			const float Variation = FMath::PerlinNoise2D(Position * 0.0022f) * 0.07f;
			const FLinearColor Moss(0.34f, 0.46f, 0.18f, 1.0f);
			const FLinearColor Meadow(0.42f, 0.54f, 0.22f, 1.0f);
			const FLinearColor Stone(0.43f, 0.44f, 0.39f, 1.0f);
			FLinearColor Color = FMath::Lerp(Moss, Meadow, FMath::Clamp((Height + 500.0f) / 2600.0f, 0.0f, 1.0f));
			Color = FMath::Lerp(Color, Stone, FMath::Clamp(RockBlend * 2.2f + HeightBlend * 0.72f, 0.0f, 1.0f));
			Color += FLinearColor(Variation, Variation * 0.85f, Variation * 0.55f, 0.0f);
			Color.A = 1.0f;
			Vertices.Emplace(Position.X, Position.Y, Height);
			Normals.Add(Normal);
			UVs.Emplace(Position.X / 900.0f, Position.Y / 900.0f);
			VertexColors.Add(Color);
			Tangents.Emplace(FVector(1.0f, 0.0f, (HeightRight - HeightLeft) / (Step * 2.0f)).GetSafeNormal(), false);
		}
	}

	for (int32 Y = 0; Y < Resolution - 1; ++Y)
	{
		for (int32 X = 0; X < Resolution - 1; ++X)
		{
			const int32 A = Y * Resolution + X;
			const int32 B = A + 1;
			const int32 C = A + Resolution;
			const int32 D = C + 1;
			Triangles.Append({ A, C, B, B, C, D });
		}
	}
	TerrainMesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UVs, VertexColors, Tangents, true);
}

void AUEGT1TechDemoEnvironment::GenerateWaterAndPaths(FRandomStream& Random)
{
	TArray<FVector> LakeVertices;
	TArray<int32> LakeTriangles;
	TArray<FVector> LakeNormals;
	TArray<FVector2D> LakeUVs;
	TArray<FLinearColor> LakeColors;
	TArray<FProcMeshTangent> LakeTangents;
	constexpr int32 LakeSegments = 96;
	LakeVertices.Reserve(LakeSegments + 2);
	LakeNormals.Reserve(LakeSegments + 2);
	LakeUVs.Reserve(LakeSegments + 2);
	LakeColors.Reserve(LakeSegments + 2);
	LakeTangents.Reserve(LakeSegments + 2);
	LakeTriangles.Reserve(LakeSegments * 3);
	LakeVertices.Emplace(LakeCenter.X, LakeCenter.Y, LakeWaterHeight);
	LakeNormals.Emplace(FVector::UpVector);
	LakeUVs.Emplace(0.5f, 0.5f);
	LakeColors.Emplace(FLinearColor::White);
	LakeTangents.Emplace(FVector::ForwardVector, false);
	for (int32 Segment = 0; Segment <= LakeSegments; ++Segment)
	{
		const float Angle = UE_TWO_PI * static_cast<float>(Segment) / static_cast<float>(LakeSegments);
		const float ShoreVariation = FMath::Sin(Angle * 3.0f + 0.4f) * 170.0f + FMath::Sin(Angle * 7.0f - 1.1f) * 95.0f;
		const float RadiusX = 3650.0f + ShoreVariation;
		const float RadiusY = 3180.0f + ShoreVariation * 0.64f;
		LakeVertices.Emplace(LakeCenter.X + FMath::Cos(Angle) * RadiusX,
			LakeCenter.Y + FMath::Sin(Angle) * RadiusY, LakeWaterHeight);
		LakeNormals.Emplace(FVector::UpVector);
		LakeUVs.Emplace(0.5f + FMath::Cos(Angle) * 0.5f, 0.5f + FMath::Sin(Angle) * 0.5f);
		LakeColors.Emplace(FLinearColor::White);
		LakeTangents.Emplace(FVector::ForwardVector, false);
	}
	for (int32 Segment = 1; Segment <= LakeSegments; ++Segment)
	{
		LakeTriangles.Append({ 0, Segment + 1, Segment });
	}
	LakeSurface->CreateMeshSection_LinearColor(0, LakeVertices, LakeTriangles, LakeNormals, LakeUVs, LakeColors, LakeTangents, false);

	for (float Y = -13100.0f; Y < 400.0f; Y += 190.0f)
	{
		const float X = TrailCenterX(Y);
		const FVector2D Position(X, Y);
		const float Z = SampleTerrainHeight(Position);
		const float Yaw = FMath::RadiansToDegrees(FMath::Atan2(TrailCenterX(Y + 50.0f) - X, 50.0f));
		const float CreekX = X + 1050.0f + FMath::Sin(Y * 0.0008f) * 280.0f;
		const FVector2D CreekPosition(CreekX, Y);
		WaterInstances->AddInstance(FTransform(FRotator(0.0f, 90.0f - Yaw, 0.0f),
			FVector(CreekX, Y, SampleTerrainHeight(CreekPosition) + 18.0f), FVector(2.5f, 1.42f, 0.035f)));
	}

	const float WaterfallTop = SampleTerrainHeight(FVector2D(0.0f, 7350.0f)) + 220.0f;
	const float WaterfallBottom = LakeWaterHeight + 80.0f;
	const float WaterfallHeight = FMath::Max(800.0f, WaterfallTop - WaterfallBottom);
	for (int32 Strand = -6; Strand <= 6; ++Strand)
	{
		const float StrandHeight = WaterfallHeight * Random.FRandRange(0.86f, 1.04f);
		WaterInstances->AddInstance(FTransform(FRotator(0.0f, Random.FRandRange(-3.0f, 3.0f), 0.0f),
			FVector(Strand * 165.0f + Random.FRandRange(-45.0f, 45.0f), 6650.0f + Random.FRandRange(-30.0f, 30.0f),
				WaterfallBottom + StrandHeight * 0.5f),
			FVector(Random.FRandRange(1.1f, 1.75f), 0.12f, StrandHeight / 100.0f)));
	}
	for (int32 Index = 0; Index < 28; ++Index)
	{
		const FVector Position(Random.FRandRange(-1050.0f, 1050.0f), Random.FRandRange(6200.0f, 6900.0f),
			WaterfallBottom + Random.FRandRange(-30.0f, 110.0f));
		FoamInstances->AddInstance(FTransform(FRotator::ZeroRotator, Position,
			FVector(Random.FRandRange(0.8f, 2.4f), Random.FRandRange(0.35f, 1.0f), Random.FRandRange(0.08f, 0.22f))));
	}
}

void AUEGT1TechDemoEnvironment::GenerateForest(FRandomStream& Random)
{
	const float HalfSize = TerrainSize * 0.5f - 650.0f;
	for (int32 Index = 0; Index < TreeCandidates; ++Index)
	{
		const FVector2D Position(Random.FRandRange(-HalfSize, HalfSize), Random.FRandRange(-HalfSize, HalfSize));
		if (IsLake(Position, 280.0f) || IsTrail(Position, 420.0f) || FVector2D::Distance(Position, FVector2D(-6200.0f, 1000.0f)) < 1050.0f)
		{
			continue;
		}
		const float NormalZ = SampleSlope(Position);
		const float Radius = Position.Size();
		const float Density = FMath::Clamp(0.78f - SmoothRange(12000.0f, 18000.0f, Radius) * 0.28f - (1.0f - NormalZ) * 0.65f, 0.12f, 0.82f);
		if (Random.FRand() > Density)
		{
			continue;
		}

		const float Ground = SampleTerrainHeight(Position);
		const float Scale = Random.FRandRange(0.68f, 1.34f);
		const float Yaw = Random.FRandRange(0.0f, 360.0f);
		TreeInstances->AddInstance(FTransform(FRotator(Random.FRandRange(-1.8f, 1.8f), Yaw, Random.FRandRange(-1.8f, 1.8f)),
			FVector(Position.X, Position.Y, Ground + 22.0f),
			FVector(Scale * Random.FRandRange(0.90f, 1.08f), Scale * Random.FRandRange(0.90f, 1.08f),
				Scale * Random.FRandRange(0.88f, 1.18f))));
		++GeneratedTreeCount;
	}
}

void AUEGT1TechDemoEnvironment::GenerateGroundDetail(FRandomStream& Random)
{
	const float HalfSize = TerrainSize * 0.5f - 450.0f;
	for (int32 Index = 0; Index < GroundDetailCandidates; ++Index)
	{
		const FVector2D Position(Random.FRandRange(-HalfSize, HalfSize), Random.FRandRange(-HalfSize, HalfSize));
		if (IsLake(Position, 60.0f) || SampleSlope(Position) < 0.76f)
		{
			continue;
		}
		const float Ground = SampleTerrainHeight(Position);
		const float TrailFade = IsTrail(Position, 120.0f) ? 0.12f : 1.0f;
		if (Random.FRand() < 0.77f * TrailFade)
		{
			const float Scale = Random.FRandRange(0.55f, 1.35f);
			GrassInstances->AddInstance(FTransform(FRotator(Random.FRandRange(-7.0f, 7.0f), Random.FRandRange(0.0f, 360.0f), 0.0f),
				FVector(Position.X, Position.Y, Ground + 28.0f * Scale), FVector(0.10f * Scale, 0.055f * Scale, 0.62f * Scale)));
		}
		if (Random.FRand() < 0.16f * TrailFade)
		{
			for (int32 Leaf = 0; Leaf < 3; ++Leaf)
			{
				FernInstances->AddInstance(FTransform(FRotator(Random.FRandRange(-10.0f, 10.0f), Random.FRandRange(0.0f, 360.0f), 0.0f),
					FVector(Position.X + Random.FRandRange(-24.0f, 24.0f), Position.Y + Random.FRandRange(-24.0f, 24.0f), Ground + 34.0f),
					FVector(0.12f, 0.045f, Random.FRandRange(0.52f, 0.92f))));
			}
		}
		if (Random.FRand() < 0.055f && Ground < 1500.0f)
		{
			FlowerInstances->AddInstance(FTransform(FRotator::ZeroRotator, FVector(Position.X, Position.Y, Ground + 54.0f), FVector(0.08f, 0.08f, 0.08f)));
		}
	}

	for (int32 Index = 0; Index < 150; ++Index)
	{
		const FVector2D Position(Random.FRandRange(-HalfSize, HalfSize), Random.FRandRange(-HalfSize, HalfSize));
		if (IsLake(Position, 100.0f) || IsTrail(Position, 160.0f))
		{
			continue;
		}
		const float Length = Random.FRandRange(260.0f, 620.0f);
		FallenLogInstances->AddInstance(FTransform(FRotator(90.0f, Random.FRandRange(0.0f, 360.0f), 0.0f),
			FVector(Position.X, Position.Y, SampleTerrainHeight(Position) + 38.0f), FVector(0.32f, 0.32f, Length / 100.0f)));
	}
}

void AUEGT1TechDemoEnvironment::GenerateRockFormations(FRandomStream& Random)
{
	const float HalfSize = TerrainSize * 0.5f - 350.0f;
	for (int32 Index = 0; Index < 900; ++Index)
	{
		const FVector2D Position(Random.FRandRange(-HalfSize, HalfSize), Random.FRandRange(-HalfSize, HalfSize));
		const float NormalZ = SampleSlope(Position);
		const float Radius = Position.Size();
		const bool bCliff = NormalZ < 0.76f || Radius > 14200.0f;
		const float Chance = bCliff ? 0.46f : 0.10f;
		if (Random.FRand() > Chance || IsLake(Position, -180.0f) || IsTrail(Position, 950.0f) ||
			FVector2D::Distance(Position, FVector2D(0.0f, -13200.0f)) < 2200.0f ||
			FVector2D::Distance(Position, FVector2D(-6200.0f, 1000.0f)) < 1600.0f)
		{
			continue;
		}
		const float Scale = bCliff ? Random.FRandRange(1.2f, 3.5f) : Random.FRandRange(0.45f, 1.45f);
		const FVector Location(Position.X, Position.Y, SampleTerrainHeight(Position) + Scale * 38.0f);
		const FTransform Transform(FRotator(Random.FRandRange(-24.0f, 24.0f), Random.FRandRange(0.0f, 360.0f), Random.FRandRange(-22.0f, 22.0f)),
			Location, FVector(Scale * Random.FRandRange(0.65f, 1.25f), Scale * Random.FRandRange(0.55f, 1.1f), Scale * Random.FRandRange(0.45f, 1.35f)));
		(bCliff ? CliffInstances : RockInstances)->AddInstance(Transform);
	}

	for (int32 Side : { -1, 1 })
	{
		for (int32 Index = 0; Index < 18; ++Index)
		{
			const FVector2D Position(Side * Random.FRandRange(1050.0f, 1850.0f), Random.FRandRange(6050.0f, 7250.0f));
			const float Scale = Random.FRandRange(2.0f, 4.4f);
			CliffInstances->AddInstance(FTransform(FRotator(Random.FRandRange(-18.0f, 18.0f), Random.FRandRange(0.0f, 360.0f), 0.0f),
				FVector(Position.X, Position.Y, SampleTerrainHeight(Position) + Scale * 45.0f), FVector(Scale, Scale * 0.7f, Scale * 1.15f)));
		}
	}
}

void AUEGT1TechDemoEnvironment::ApplyMaterial(UPrimitiveComponent* Component, EUEGT1VisualMaterial Style,
	const FLinearColor& Color, float Roughness, float Specular, float Metallic, float EmissiveStrength)
{
	UEGT1VisualMaterials::Apply(this, Component, FallbackMaterial, Style, Color, Roughness, Specular, Metallic, EmissiveStrength);
}

void AUEGT1TechDemoEnvironment::ApplyMaterials()
{
	ApplyMaterial(TerrainMesh, EUEGT1VisualMaterial::TechTerrain, FLinearColor(0.13f, 0.25f, 0.10f), 0.91f, 0.16f);
	ApplyMaterial(LakeSurface, EUEGT1VisualMaterial::TechWater, FLinearColor(0.009f, 0.052f, 0.044f), 0.24f, 0.38f);
	ApplyMaterial(WaterInstances, EUEGT1VisualMaterial::TechWater, FLinearColor(0.008f, 0.046f, 0.040f), 0.27f, 0.34f);
	ApplyMaterial(PathInstances, EUEGT1VisualMaterial::TechSurface, FLinearColor(0.26f, 0.19f, 0.095f), 0.96f, 0.12f);
	ApplyMaterial(TrunkInstances, EUEGT1VisualMaterial::TechSurface, FLinearColor(0.095f, 0.045f, 0.021f), 0.94f, 0.12f);
	ApplyMaterial(FallenLogInstances, EUEGT1VisualMaterial::TechSurface, FLinearColor(0.07f, 0.032f, 0.015f), 0.97f, 0.10f);
	ApplyMaterial(CanopyInstances, EUEGT1VisualMaterial::TechFoliage, FLinearColor(0.024f, 0.12f, 0.030f), 0.78f, 0.24f);
	ApplyMaterial(PineInstances, EUEGT1VisualMaterial::TechFoliage, FLinearColor(0.010f, 0.060f, 0.034f), 0.82f, 0.20f);
	ApplyMaterial(GrassInstances, EUEGT1VisualMaterial::TechFoliage, FLinearColor(0.025f, 0.105f, 0.018f), 0.86f, 0.18f);
	ApplyMaterial(FernInstances, EUEGT1VisualMaterial::TechFoliage, FLinearColor(0.014f, 0.095f, 0.026f), 0.80f, 0.20f);
	ApplyMaterial(FlowerInstances, EUEGT1VisualMaterial::TechFoliage, FLinearColor(0.95f, 0.52f, 0.13f), 0.64f, 0.24f);
	ApplyMaterial(RockInstances, EUEGT1VisualMaterial::TechSurface, FLinearColor(0.22f, 0.245f, 0.22f), 0.98f, 0.10f);
	ApplyMaterial(CliffInstances, EUEGT1VisualMaterial::TechSurface, FLinearColor(0.17f, 0.19f, 0.18f), 0.99f, 0.08f);
	ApplyMaterial(FoamInstances, EUEGT1VisualMaterial::Glow, FLinearColor(0.48f, 0.86f, 0.78f), 0.15f, 0.88f, 0.0f, 0.15f);
}

int32 AUEGT1TechDemoEnvironment::GetGeneratedInstanceCount() const
{
	int32 Count = 0;
	for (const UHierarchicalInstancedStaticMeshComponent* Component : { WaterInstances, PathInstances, TreeInstances, TrunkInstances, CanopyInstances,
		PineInstances, RockInstances, CliffInstances, GrassInstances, FernInstances, FlowerInstances, FallenLogInstances, FoamInstances })
	{
		Count += Component->GetInstanceCount();
	}
	return Count;
}

void AUEGT1TechDemoEnvironment::ApplyGraphicsSettings()
{
	const UUEGT1GameUserSettings* Settings = UUEGT1GameUserSettings::Get();
	const bool bFoliage = !Settings || Settings->IsFeatureEnabled(EUEGT1GraphicsFeature::Foliage);
	for (UHierarchicalInstancedStaticMeshComponent* Component : { TreeInstances, TrunkInstances, CanopyInstances, PineInstances, GrassInstances,
		FernInstances, FlowerInstances, FallenLogInstances })
	{
		Component->SetVisibility(bFoliage, true);
	}
}
