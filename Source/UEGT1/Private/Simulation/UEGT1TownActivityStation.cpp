#include "Simulation/UEGT1TownActivityStation.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Simulation/UEGT1TownSimulationSubsystem.h"
#include "UEGT1LogChannels.h"
#include "UObject/ConstructorHelpers.h"
#include "World/UEGT1Palette.h"
#include "World/UEGT1VisualMaterials.h"

AUEGT1TownActivityStation::AUEGT1TownActivityStation()
{
	PrimaryActorTick.bCanEverTick = false;
	InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionVolume"));
	SetRootComponent(InteractionVolume);
	InteractionVolume->SetBoxExtent(FVector(85.0f, 85.0f, 105.0f));
	InteractionVolume->SetRelativeLocation(FVector(0.0f, 0.0f, 105.0f));
	InteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionVolume->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	InteractionVolume->SetCanEverAffectNavigation(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	MarkerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ActivityMarker"));
	MarkerMesh->SetupAttachment(InteractionVolume);
	MarkerMesh->SetStaticMesh(CubeFinder.Object);
	MarkerMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 86.0f));
	MarkerMesh->SetRelativeScale3D(FVector(0.18f, 0.18f, 0.18f));
	MarkerMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	MarkerMesh->SetCanEverAffectNavigation(false);
}

void AUEGT1TownActivityStation::BeginPlay()
{
	Super::BeginPlay();
	UEGT1VisualMaterials::Apply(this, MarkerMesh, nullptr, EUEGT1VisualMaterial::Glow,
		UEGT1Palette::Signal, 0.2f, 0.8f, 0.0f, 10.0f);
}

void AUEGT1TownActivityStation::Configure(FName InVenueId, EUEGT1SimActionType InAction,
	const FString& InVenueName, const FString& InJobTitle, float InHourlyRate)
{
	VenueId = InVenueId;
	Action = InAction;
	VenueName = InVenueName;
	JobTitle = InJobTitle;
	HourlyRate = InHourlyRate;
}

bool AUEGT1TownActivityStation::CanInteract_Implementation(APawn* InstigatorPawn) const
{
	const UUEGT1TownSimulationSubsystem* Simulation = GetWorld()->GetSubsystem<UUEGT1TownSimulationSubsystem>();
	return Simulation && Simulation->IsSimulationRunning();
}

FText AUEGT1TownActivityStation::GetInteractionPrompt_Implementation(APawn* InstigatorPawn) const
{
	const UUEGT1TownSimulationSubsystem* Simulation = GetWorld()->GetSubsystem<UUEGT1TownSimulationSubsystem>();
	FString Reason;
	if (!Simulation || !Simulation->CanPlayerPerformActivity(Action, VenueId, Reason))
	{
		return FText::FromString(FString::Printf(TEXT("[E] %s"), Reason.IsEmpty() ? TEXT("Activity unavailable") : *Reason));
	}
	return FText::FromString(FString::Printf(TEXT("[E] %s"), *MakeAvailablePrompt()));
}

void AUEGT1TownActivityStation::Interact_Implementation(APawn* InstigatorPawn)
{
	if (UUEGT1TownSimulationSubsystem* Simulation = GetWorld()->GetSubsystem<UUEGT1TownSimulationSubsystem>())
	{
		FString Result;
		const bool bCompleted = Simulation->PerformPlayerActivity(Action, VenueId, Result);
		UE_LOG(LogUEGT1, Display, TEXT("Player activity: Action=%s Venue=%s Success=%s Result=%s"),
			LexToString(Action), *VenueId.ToString(), bCompleted ? TEXT("true") : TEXT("false"), *Result);
	}
}

void AUEGT1TownActivityStation::SetInteractionFocus_Implementation(bool bFocused)
{
	MarkerMesh->SetRelativeScale3D(bFocused ? FVector(0.30f) : FVector(0.18f));
}

FString AUEGT1TownActivityStation::MakeAvailablePrompt() const
{
	switch (Action)
	{
	case EUEGT1SimActionType::Sleep: return TEXT("Sleep here (6 hours)");
	case EUEGT1SimActionType::Eat: return VenueId.ToString().StartsWith(TEXT("Home"))
		? TEXT("Cook a meal ($1)") : FString::Printf(TEXT("Eat at %s ($12)"), *VenueName);
	case EUEGT1SimActionType::Hygiene: return TEXT("Take a shower (50 minutes)");
	case EUEGT1SimActionType::Socialize: return FString::Printf(TEXT("Socialize at %s"), *VenueName);
	case EUEGT1SimActionType::Work: return FString::Printf(TEXT("Work one hour as %s at %s ($%.0f/hr)"),
		JobTitle.IsEmpty() ? TEXT("staff") : *JobTitle, *VenueName, HourlyRate);
	default: return TEXT("Use activity");
	}
}
