#pragma once

#include "CoreMinimal.h"
#include "Simulation/UEGT1TownSimulationTypes.h"
#include "UEGT1TownGeneration.generated.h"

USTRUCT(BlueprintType)
struct FUEGT1GeneratedStreet
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Town Generation") FVector Start = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "Town Generation") FVector End = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "Town Generation") float Width = 520.0f;
};

USTRUCT(BlueprintType)
struct FUEGT1GeneratedTownLot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Town Generation") FName LotId;
	UPROPERTY(BlueprintReadOnly, Category = "Town Generation") FString DisplayName;
	UPROPERTY(BlueprintReadOnly, Category = "Town Generation") FString JobTitle;
	UPROPERTY(BlueprintReadOnly, Category = "Town Generation") FString BusinessType;
	UPROPERTY(BlueprintReadOnly, Category = "Town Generation") EUEGT1TownVenueType VenueType = EUEGT1TownVenueType::Home;
	UPROPERTY(BlueprintReadOnly, Category = "Town Generation") FVector Center = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "Town Generation") FVector AccessPoint = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "Town Generation") FRotator FacingRotation = FRotator::ZeroRotator;
	UPROPERTY(BlueprintReadOnly, Category = "Town Generation") FVector EntranceLocation = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "Town Generation") FVector InteriorEntryLocation = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "Town Generation") FVector KitchenLocation = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "Town Generation") FVector BathroomLocation = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "Town Generation") FVector ShowerLocation = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "Town Generation") FVector ActivityLocation = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "Town Generation") TArray<FVector> AmenityBedLocations;
	UPROPERTY(BlueprintReadOnly, Category = "Town Generation") FVector2D Footprint = FVector2D(900.0f, 760.0f);
	UPROPERTY(BlueprintReadOnly, Category = "Town Generation") float BuildingHeight = 700.0f;
	UPROPERTY(BlueprintReadOnly, Category = "Town Generation") int32 Capacity = 1;
	UPROPERTY(BlueprintReadOnly, Category = "Town Generation") float HourlyRate = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "Town Generation") float OpeningHour = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "Town Generation") float ClosingHour = 24.0f;
	UPROPERTY(BlueprintReadOnly, Category = "Town Generation") TArray<FUEGT1TownBedState> Beds;
};

USTRUCT(BlueprintType)
struct FUEGT1GeneratedTownLayout
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Town Generation") int32 Seed = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Town Generation") TArray<FUEGT1GeneratedStreet> Streets;
	UPROPERTY(BlueprintReadOnly, Category = "Town Generation") TArray<FVector> Intersections;
	UPROPERTY(BlueprintReadOnly, Category = "Town Generation") TArray<FUEGT1GeneratedTownLot> Lots;
	UPROPERTY(BlueprintReadOnly, Category = "Town Generation") FVector2D BoundsMin = FVector2D::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "Town Generation") FVector2D BoundsMax = FVector2D::ZeroVector;
};

namespace UEGT1TownGeneration
{
	UEGT1_API FUEGT1GeneratedTownLayout Generate(int32 Seed, const FUEGT1SimulationTuning& Tuning);
	UEGT1_API bool IsFullyConnected(const FUEGT1GeneratedTownLayout& Layout);
	UEGT1_API TArray<FVector> BuildStreetPath(const FVector& Start, const FVector& StartAccess,
		const FVector& DestinationAccess, const FVector& Destination);
	UEGT1_API TArray<FVector> BuildVenuePath(const FVector& Start, const FUEGT1TownVenueState* CurrentVenue,
		const FUEGT1TownVenueState& DestinationVenue, const FVector& ActivityLocation);
}
