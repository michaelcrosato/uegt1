#include "Interaction/UEGT1InteractionComponent.h"

#include "DrawDebugHelpers.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Interaction/UEGT1Interactable.h"
#include "UEGT1LogChannels.h"

static TAutoConsoleVariable<int32> CVarUEGT1DrawInteraction(
	TEXT("uegt1.Debug.DrawInteraction"),
	0,
	TEXT("Draw the local player's interaction trace. 0=off, 1=on."),
	ECVF_Cheat);

UUEGT1InteractionComponent::UUEGT1InteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.05f;
}

void UUEGT1InteractionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UUEGT1InteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SetFocusedActor(nullptr);
	Super::EndPlay(EndPlayReason);
}

void UUEGT1InteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	RefreshFocus();
}

void UUEGT1InteractionComponent::RefreshFocus()
{
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	APlayerController* PlayerController = OwnerPawn ? Cast<APlayerController>(OwnerPawn->GetController()) : nullptr;
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		SetFocusedActor(nullptr);
		return;
	}

	FVector ViewLocation;
	FRotator ViewRotation;
	PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
	const FVector TraceEnd = ViewLocation + ViewRotation.Vector() * TraceDistance;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(UEGT1Interaction), false, GetOwner());
	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(HitResult, ViewLocation, TraceEnd, ECC_Visibility, QueryParams);

	AActor* Candidate = HitResult.GetActor();
	if (!Candidate || !Candidate->GetClass()->ImplementsInterface(UUEGT1Interactable::StaticClass()) ||
		!IUEGT1Interactable::Execute_CanInteract(Candidate, const_cast<APawn*>(OwnerPawn)))
	{
		Candidate = nullptr;
	}

	SetFocusedActor(Candidate);

	if (CVarUEGT1DrawInteraction.GetValueOnGameThread() != 0)
	{
		DrawDebugLine(GetWorld(), ViewLocation, HitResult.bBlockingHit ? HitResult.ImpactPoint : TraceEnd,
			Candidate ? FColor::Cyan : FColor::Orange, false, 0.06f, 0, 1.0f);
	}
}

void UUEGT1InteractionComponent::SetFocusedActor(AActor* NewFocusedActor)
{
	if (FocusedActor.Get() == NewFocusedActor)
	{
		return;
	}

	if (AActor* PreviousActor = FocusedActor.Get())
	{
		IUEGT1Interactable::Execute_SetInteractionFocus(PreviousActor, false);
	}

	FocusedActor = NewFocusedActor;
	if (NewFocusedActor)
	{
		IUEGT1Interactable::Execute_SetInteractionFocus(NewFocusedActor, true);
	}
}

bool UUEGT1InteractionComponent::HasValidFocus() const
{
	return FocusedActor.IsValid();
}

FText UUEGT1InteractionComponent::GetFocusedPrompt() const
{
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	return FocusedActor.IsValid()
		? IUEGT1Interactable::Execute_GetInteractionPrompt(FocusedActor.Get(), const_cast<APawn*>(OwnerPawn))
		: FText::GetEmpty();
}

void UUEGT1InteractionComponent::TryInteract()
{
	AActor* Target = FocusedActor.Get();
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!Target || !OwnerPawn || !IUEGT1Interactable::Execute_CanInteract(Target, OwnerPawn))
	{
		return;
	}

	UE_LOG(LogUEGT1, Log, TEXT("Interaction requested: Player=%s Target=%s"), *GetNameSafe(OwnerPawn), *GetNameSafe(Target));
	IUEGT1Interactable::Execute_Interact(Target, OwnerPawn);
}
