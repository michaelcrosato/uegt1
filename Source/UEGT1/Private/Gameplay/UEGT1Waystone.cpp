#include "Gameplay/UEGT1Waystone.h"

#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Gameplay/UEGT1MilestoneGameState.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "UEGT1LogChannels.h"
#include "World/UEGT1Palette.h"
#include "World/UEGT1VisualMaterials.h"

AUEGT1Waystone::AUEGT1Waystone()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeFinder(TEXT("/Engine/BasicShapes/Cone.Cone"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	ConeMesh = ConeFinder.Object;
	ShapeMaterial = MaterialFinder.Object;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Base"));
	BaseMesh->SetupAttachment(SceneRoot);
	BaseMesh->SetStaticMesh(CylinderFinder.Object);
	BaseMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 22.0f));
	BaseMesh->SetRelativeScale3D(FVector(1.45f, 1.45f, 0.22f));

	PedestalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pedestal"));
	PedestalMesh->SetupAttachment(SceneRoot);
	PedestalMesh->SetStaticMesh(CubeFinder.Object);
	PedestalMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 105.0f));
	PedestalMesh->SetRelativeScale3D(FVector(0.58f, 0.58f, 1.25f));

	for (int32 Index = 0; Index < 3; ++Index)
	{
		UStaticMeshComponent* Shard = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("Shard%d"), Index + 1));
		Shard->SetupAttachment(SceneRoot);
		Shard->SetStaticMesh(ConeMesh);
		const float Angle = Index * 120.0f;
		const FVector Offset(FMath::Cos(FMath::DegreesToRadians(Angle)) * 58.0f, FMath::Sin(FMath::DegreesToRadians(Angle)) * 58.0f, 205.0f);
		Shard->SetRelativeLocation(Offset);
		Shard->SetRelativeRotation(FRotator(Index == 1 ? 180.0f : 0.0f, Angle, Index == 2 ? 180.0f : 0.0f));
		Shard->SetRelativeScale3D(FVector(0.38f, 0.38f, 1.1f));
		ShardMeshes.Add(Shard);
	}

	SignalLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("SignalLight"));
	SignalLight->SetupAttachment(SceneRoot);
	SignalLight->SetRelativeLocation(FVector(0.0f, 0.0f, 235.0f));
	SignalLight->SetAttenuationRadius(900.0f);
	SignalLight->SetCastShadows(false);
	SignalLight->SetIntensity(950.0f);
}

void AUEGT1Waystone::InitializeWaystone(FName InWaystoneId)
{
	WaystoneId = InWaystoneId;
#if WITH_EDITOR
	SetActorLabel(FString::Printf(TEXT("Waystone_%s"), *WaystoneId.ToString()));
#endif
}

void AUEGT1Waystone::BeginPlay()
{
	Super::BeginPlay();

	DynamicMaterials.Reset();
	DynamicMaterials.Add(CreateColorMaterial(BaseMesh, UEGT1Palette::Stone));
	DynamicMaterials.Add(CreateColorMaterial(PedestalMesh, UEGT1Palette::DeepForest));
	for (UStaticMeshComponent* Shard : ShardMeshes)
	{
		DynamicMaterials.Add(CreateColorMaterial(Shard, UEGT1Palette::Amber, true));
	}

	if (AUEGT1MilestoneGameState* MilestoneState = GetWorld()->GetGameState<AUEGT1MilestoneGameState>())
	{
		MilestoneState->RegisterWaystone(WaystoneId);
	}
	ApplyVisualState();
}

UMaterialInstanceDynamic* AUEGT1Waystone::CreateColorMaterial(UPrimitiveComponent* Component, const FLinearColor& Color, bool bGlow)
{
	return UEGT1VisualMaterials::Apply(this, Component, ShapeMaterial,
		bGlow ? EUEGT1VisualMaterial::Glow : EUEGT1VisualMaterial::Surface,
		Color, bGlow ? 0.16f : 0.88f, bGlow ? 0.84f : 0.20f, 0.0f, bGlow ? 2.8f : 0.0f);
}

void AUEGT1Waystone::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	AnimationTime += DeltaSeconds;

	const float RotationSpeed = bActivated ? 34.0f : 9.0f;
	for (int32 Index = 0; Index < ShardMeshes.Num(); ++Index)
	{
		UStaticMeshComponent* Shard = ShardMeshes[Index];
		Shard->AddLocalRotation(FRotator(0.0f, RotationSpeed * DeltaSeconds * (Index % 2 == 0 ? 1.0f : -1.0f), 0.0f));
		FVector Location = Shard->GetRelativeLocation();
		Location.Z = 205.0f + FMath::Sin(AnimationTime * 2.1f + Index * 2.0f) * (bActivated ? 14.0f : 5.0f);
		Shard->SetRelativeLocation(Location);
	}

	const float Pulse = 0.82f + FMath::Sin(AnimationTime * (bActivated ? 4.0f : 2.0f)) * 0.18f;
	SignalLight->SetIntensity((bActivated ? 4600.0f : 950.0f) * Pulse + (bFocused ? 900.0f : 0.0f));
}

bool AUEGT1Waystone::CanInteract_Implementation(APawn* InstigatorPawn) const
{
	return !bActivated;
}

FText AUEGT1Waystone::GetInteractionPrompt_Implementation(APawn* InstigatorPawn) const
{
	return bActivated
		? FText::FromString(TEXT("Waystone stabilized"))
		: FText::FromString(TEXT("[E] Stabilize Waystone"));
}

void AUEGT1Waystone::Interact_Implementation(APawn* InstigatorPawn)
{
	if (bActivated)
	{
		return;
	}

	if (AUEGT1MilestoneGameState* MilestoneState = GetWorld()->GetGameState<AUEGT1MilestoneGameState>())
	{
		bActivated = MilestoneState->ActivateWaystone(WaystoneId);
		if (bActivated)
		{
			ApplyVisualState();
		}
	}
}

void AUEGT1Waystone::SetInteractionFocus_Implementation(bool bNewFocused)
{
	bFocused = bNewFocused;
}

void AUEGT1Waystone::ApplyVisualState()
{
	const FLinearColor ShardColor = bActivated ? UEGT1Palette::Signal : UEGT1Palette::Amber;
	for (int32 Index = 2; Index < DynamicMaterials.Num(); ++Index)
	{
		if (DynamicMaterials[Index])
		{
			DynamicMaterials[Index]->SetVectorParameterValue(TEXT("Color"), ShardColor);
		}
	}
	SignalLight->SetLightColor(ShardColor);
}
