#include "Gameplay/UEGT1Sanctuary.h"

#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Gameplay/UEGT1MilestoneGameState.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "UEGT1LogChannels.h"
#include "World/UEGT1Palette.h"
#include "World/UEGT1VisualMaterials.h"

AUEGT1Sanctuary::AUEGT1Sanctuary()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	ShapeMaterial = MaterialFinder.Object;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Platform"));
	PlatformMesh->SetupAttachment(SceneRoot);
	PlatformMesh->SetStaticMesh(CylinderFinder.Object);
	PlatformMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 35.0f));
	PlatformMesh->SetRelativeScale3D(FVector(5.8f, 5.8f, 0.35f));

	MotionRoot = CreateDefaultSubobject<USceneComponent>(TEXT("MotionRoot"));
	MotionRoot->SetupAttachment(SceneRoot);
	MotionRoot->SetRelativeLocation(FVector(0.0f, 0.0f, 170.0f));

	for (int32 Index = 0; Index < 3; ++Index)
	{
		UStaticMeshComponent* Slab = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("OrbitingSlab%d"), Index + 1));
		Slab->SetupAttachment(MotionRoot);
		const float Angle = Index * 120.0f;
		Slab->SetStaticMesh(CubeFinder.Object);
		Slab->SetRelativeLocation(FVector(FMath::Cos(FMath::DegreesToRadians(Angle)) * 260.0f, FMath::Sin(FMath::DegreesToRadians(Angle)) * 260.0f, Index * 35.0f));
		Slab->SetRelativeRotation(FRotator(0.0f, Angle + 30.0f, 18.0f));
		Slab->SetRelativeScale3D(FVector(2.1f, 0.24f, 0.18f));
		Slab->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		OrbitingSlabs.Add(Slab);
	}

	CoreMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Core"));
	CoreMesh->SetupAttachment(SceneRoot);
	CoreMesh->SetStaticMesh(SphereFinder.Object);
	CoreMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 180.0f));
	CoreMesh->SetRelativeScale3D(FVector(0.9f));
	CoreMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	CoreLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("CoreLight"));
	CoreLight->SetupAttachment(CoreMesh);
	CoreLight->SetAttenuationRadius(1500.0f);
	CoreLight->SetCastShadows(false);
}

void AUEGT1Sanctuary::BeginPlay()
{
	Super::BeginPlay();

	CreateColorMaterial(PlatformMesh, UEGT1Palette::Stone);
	for (UStaticMeshComponent* Slab : OrbitingSlabs)
	{
		CreateColorMaterial(Slab, UEGT1Palette::DeepForest);
	}
	CoreMaterial = CreateColorMaterial(CoreMesh, UEGT1Palette::Amber, true);

	if (AUEGT1MilestoneGameState* MilestoneState = GetWorld()->GetGameState<AUEGT1MilestoneGameState>())
	{
		MilestoneState->OnObjectiveProgress.AddDynamic(this, &AUEGT1Sanctuary::HandleObjectiveProgress);
		HandleObjectiveProgress(MilestoneState->GetActivatedCount(), MilestoneState->GetTotalCount(), MilestoneState->IsMilestoneComplete());
	}
}

UMaterialInstanceDynamic* AUEGT1Sanctuary::CreateColorMaterial(UPrimitiveComponent* Component, const FLinearColor& Color, bool bGlow)
{
	return UEGT1VisualMaterials::Apply(this, Component, ShapeMaterial,
		bGlow ? EUEGT1VisualMaterial::Glow : EUEGT1VisualMaterial::Surface,
		Color, bGlow ? 0.14f : 0.90f, bGlow ? 0.88f : 0.18f, 0.0f, bGlow ? 3.6f : 0.0f);
}

void AUEGT1Sanctuary::HandleObjectiveProgress(int32 ActivatedCount, int32 TotalCount, bool bComplete)
{
	if (!bRestored && bComplete)
	{
		UE_LOG(LogUEGT1, Display, TEXT("Signal Grove milestone complete: sanctuary restored."));
	}
	bRestored = bComplete;
	ApplyVisualState();
}

void AUEGT1Sanctuary::ApplyVisualState()
{
	const FLinearColor Color = bRestored ? UEGT1Palette::Signal : UEGT1Palette::Amber;
	if (CoreMaterial)
	{
		CoreMaterial->SetVectorParameterValue(TEXT("Color"), Color);
	}
	CoreLight->SetLightColor(Color);
	CoreLight->SetIntensity(bRestored ? 4200.0f : 900.0f);
}

void AUEGT1Sanctuary::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	AnimationTime += DeltaSeconds;

	MotionRoot->AddLocalRotation(FRotator(0.0f, DeltaSeconds * (bRestored ? 24.0f : 5.0f), 0.0f));
	FVector CoreLocation = CoreMesh->GetRelativeLocation();
	CoreLocation.Z = (bRestored ? 270.0f : 180.0f) + FMath::Sin(AnimationTime * (bRestored ? 2.6f : 1.2f)) * (bRestored ? 24.0f : 8.0f);
	CoreMesh->SetRelativeLocation(FMath::VInterpTo(CoreMesh->GetRelativeLocation(), CoreLocation, DeltaSeconds, 2.5f));
}
