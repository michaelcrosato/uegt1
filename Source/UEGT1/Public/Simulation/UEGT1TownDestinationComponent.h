#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Simulation/UEGT1TownSimulationTypes.h"
#include "UEGT1TownDestinationComponent.generated.h"

struct FUEGT1GeneratedTownLot;

UCLASS(ClassGroup = (UEGT1), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class UEGT1_API UUEGT1TownDestinationComponent final : public USceneComponent
{
	GENERATED_BODY()

public:
	UUEGT1TownDestinationComponent();
	void ConfigureFromGeneratedLot(const FUEGT1GeneratedTownLot& Lot);
	FUEGT1TownVenueState MakeVenueState() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Town Destination")
	FName VenueId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Town Destination")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Town Destination") FString JobTitle;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Town Destination") FString BusinessType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Town Destination")
	EUEGT1TownVenueType VenueType = EUEGT1TownVenueType::Home;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Town Destination", meta = (ClampMin = "0"))
	int32 Capacity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Town Destination|Economy", meta = (ClampMin = "0.0"))
	float HourlyRate = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Town Destination", meta = (ClampMin = "0.0", ClampMax = "24.0"))
	float OpeningHour = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Town Destination", meta = (ClampMin = "0.0", ClampMax = "24.0"))
	float ClosingHour = 24.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Town Destination")
	FVector AccessPoint = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Town Destination|Interior") FVector EntranceLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Town Destination|Interior") FVector InteriorEntryLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Town Destination|Interior") FVector KitchenLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Town Destination|Interior") FVector BathroomLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Town Destination|Interior") FVector ShowerLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Town Destination|Interior") FVector ActivityLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Town Destination|Interior") TArray<FVector> AmenityBedLocations;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Town Destination")
	bool bReachable = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Town Destination|Housing")
	TArray<FUEGT1TownBedState> Beds;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
