#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/UEGT1Interactable.h"
#include "Simulation/UEGT1TownSimulationTypes.h"
#include "UEGT1TownActivityStation.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class UEGT1_API AUEGT1TownActivityStation final : public AActor, public IUEGT1Interactable
{
	GENERATED_BODY()

public:
	AUEGT1TownActivityStation();
	void Configure(FName InVenueId, EUEGT1SimActionType InAction, const FString& InVenueName,
		const FString& InJobTitle, float InHourlyRate);

	FName GetVenueId() const { return VenueId; }
	EUEGT1SimActionType GetAction() const { return Action; }

	virtual bool CanInteract_Implementation(APawn* InstigatorPawn) const override;
	virtual FText GetInteractionPrompt_Implementation(APawn* InstigatorPawn) const override;
	virtual void Interact_Implementation(APawn* InstigatorPawn) override;
	virtual void SetInteractionFocus_Implementation(bool bFocused) override;

protected:
	virtual void BeginPlay() override;

private:
	FString MakeAvailablePrompt() const;

	UPROPERTY(VisibleAnywhere) TObjectPtr<UBoxComponent> InteractionVolume;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> MarkerMesh;
	UPROPERTY() FName VenueId;
	UPROPERTY() EUEGT1SimActionType Action = EUEGT1SimActionType::Idle;
	UPROPERTY() FString VenueName;
	UPROPERTY() FString JobTitle;
	UPROPERTY() float HourlyRate = 0.0f;
};
