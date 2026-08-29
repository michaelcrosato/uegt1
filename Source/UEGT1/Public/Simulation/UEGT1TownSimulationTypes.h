#pragma once

#include "CoreMinimal.h"
#include "UEGT1TownSimulationTypes.generated.h"

UENUM(BlueprintType)
enum class EUEGT1TownVenueType : uint8
{
	Home,
	FoodVenue,
	Workplace,
	SocialVenue,
	Park
};

UENUM(BlueprintType)
enum class EUEGT1SimActionType : uint8
{
	Idle,
	Travel,
	Sleep,
	Eat,
	Hygiene,
	Socialize,
	Work
};

UENUM(BlueprintType)
enum class EUEGT1HouseholdRelationship : uint8
{
	Family,
	Friend,
	Roommate
};

UEGT1_API const TCHAR* LexToString(EUEGT1TownVenueType VenueType);
UEGT1_API const TCHAR* LexToString(EUEGT1SimActionType ActionType);
UEGT1_API const TCHAR* LexToString(EUEGT1HouseholdRelationship Relationship);

USTRUCT(BlueprintType)
struct FUEGT1NeedState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Simulation|Needs", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Energy = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Simulation|Needs", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Hunger = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Simulation|Needs", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Hygiene = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Simulation|Needs", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Social = 1.0f;

	void Clamp();
};

USTRUCT(BlueprintType)
struct FUEGT1DailySchedule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Simulation|Schedule", meta = (ClampMin = "0.0", ClampMax = "24.0"))
	float WorkStartHour = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Simulation|Schedule", meta = (ClampMin = "0.0", ClampMax = "24.0"))
	float WorkEndHour = 17.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Simulation|Schedule", meta = (ClampMin = "0.0", ClampMax = "24.0"))
	float SleepStartHour = 22.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Simulation|Schedule", meta = (ClampMin = "0.0", ClampMax = "24.0"))
	float SleepEndHour = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Simulation|Schedule")
	uint8 WorkdayMask = 0x1f;

	bool IsWorkTime(int32 DayIndex, float Hour) const;
	bool IsSleepTime(float Hour) const;
};

USTRUCT(BlueprintType)
struct FUEGT1ActionDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Action")
	EUEGT1SimActionType Action = EUEGT1SimActionType::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Action")
	EUEGT1TownVenueType VenueType = EUEGT1TownVenueType::Park;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Action", meta = (ClampMin = "1.0"))
	float DurationMinutes = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Action", meta = (ClampMin = "0.0"))
	float Cost = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Action", meta = (ClampMin = "0.0"))
	float Earnings = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Action")
	FUEGT1NeedState NeedEffects = { 0.0f, 0.0f, 0.0f, 0.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Action")
	float BaseUtility = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Action", meta = (ClampMin = "0.0"))
	float NeedWeight = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Action")
	bool bPreferAssignedDestination = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Action")
	bool bRequiresWorkSchedule = false;
};

USTRUCT(BlueprintType)
struct FUEGT1JobDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Jobs")
	FName JobId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Jobs")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Jobs")
	FString BusinessName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Jobs")
	FString BusinessType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Jobs", meta = (ClampMin = "0.0", ClampMax = "24.0"))
	float OpeningHour = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Jobs", meta = (ClampMin = "0.0", ClampMax = "24.0"))
	float ClosingHour = 22.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Jobs", meta = (ClampMin = "0.0"))
	float HourlyRate = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Jobs", meta = (ClampMin = "1"))
	int32 Capacity = 5;
};

USTRUCT(BlueprintType)
struct FUEGT1SimulationTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Time") float StartHour = 6.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Time") int32 CalendarStartYear = 2026;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Time") int32 CalendarStartMonth = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Time") int32 CalendarStartDay = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Time") float SimMinutesPerRealSecond = 2.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Time") float SimulationStepMinutes = 5.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Time") float SimulationBatchIntervalRealSeconds = 0.25f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Movement", meta = (ClampMin = "1.0")) float CitizenWalkSpeedCentimetersPerSecond = 440.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Needs") float EnergyDecayPerHour = 0.038f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Needs") float HungerDecayPerHour = 0.060f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Needs") float HygieneDecayPerHour = 0.030f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Needs") float SocialDecayPerHour = 0.025f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Needs") float CriticalNeedThreshold = 0.30f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Economy") float StartingMoneyMinimum = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Economy") float StartingMoneyMaximum = 65.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Economy") float BrokeThreshold = 16.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Economy") float PlayerStartingMoney = 40.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Decision") float TravelUtilityCostPerMeter = 0.12f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Decision") float ReplanIntervalMinutes = 15.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Decision", meta = (ClampMin = "1.0")) float WorkDecisionIntervalMinutes = 60.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Presentation", meta = (ClampMin = "0.0")) float ThoughtBubbleVisibleDistance = 3000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Presentation", meta = (ClampMin = "1")) int32 MaxVisibleThoughtBubbles = 12;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Venues") int32 HomeCapacity = 4;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Venues") int32 FoodVenueCapacity = 8;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Venues") int32 WorkplaceCapacity = 10;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Venues") int32 SocialVenueCapacity = 8;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Venues") int32 ParkCapacity = 25;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Venues") float FoodVenueOpenHour = 6.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Venues") float FoodVenueCloseHour = 22.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Venues") float WorkplaceOpenHour = 7.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Venues") float WorkplaceCloseHour = 20.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Venues") float SocialVenueOpenHour = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Venues") float SocialVenueCloseHour = 24.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Action") TArray<FUEGT1ActionDefinition> ActionDefinitions;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation|Jobs") TArray<FUEGT1JobDefinition> JobDefinitions;

	float GetTravelSpeedCentimetersPerSimulationMinute() const
	{
		return CitizenWalkSpeedCentimetersPerSecond / FMath::Max(SimMinutesPerRealSecond, KINDA_SMALL_NUMBER);
	}
};

USTRUCT(BlueprintType)
struct FUEGT1TownBedState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Simulation|Housing") FName BedId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Simulation|Housing") FVector WorldLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Simulation|Housing") FRotator Rotation = FRotator::ZeroRotator;
};

USTRUCT(BlueprintType)
struct FUEGT1TownVenueState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Simulation|Venue") FName VenueId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Simulation|Venue") FString DisplayName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Simulation|Venue") FString JobTitle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Simulation|Venue") FString BusinessType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Simulation|Venue") EUEGT1TownVenueType VenueType = EUEGT1TownVenueType::Home;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Simulation|Venue") FVector WorldLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Simulation|Venue") FVector AccessPoint = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Simulation|Interior") FVector EntranceLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Simulation|Interior") FVector InteriorEntryLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Simulation|Interior") FVector KitchenLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Simulation|Interior") FVector BathroomLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Simulation|Interior") FVector ShowerLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Simulation|Interior") FVector ActivityLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Simulation|Interior") TArray<FVector> AmenityBedLocations;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Simulation|Venue") float OpeningHour = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Simulation|Venue") float ClosingHour = 24.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Simulation|Venue") int32 Capacity = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Simulation|Economy") float HourlyRate = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Simulation|Venue") bool bReachable = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Simulation|Venue") TArray<FName> Reservations;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Simulation|Housing") TArray<FUEGT1TownBedState> Beds;
	// Reserved namespaced state for later business, ownership, inventory, or crime modules.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Simulation|Extensions") TMap<FName, float> ExtensionValues;

	bool IsOpen(float Hour) const;
	bool IsOpenForDuration(float StartHour, float DurationMinutes) const;
	bool HasCapacityFor(FName NpcId) const;
};

USTRUCT(BlueprintType)
struct FUEGT1HouseholdRelationshipState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Relationships") FName OtherNpcId;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Relationships") EUEGT1HouseholdRelationship Relationship = EUEGT1HouseholdRelationship::Roommate;
};

USTRUCT(BlueprintType)
struct FUEGT1ActionUtilityScore
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Decision") EUEGT1SimActionType Action = EUEGT1SimActionType::Idle;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Decision") FName DestinationId;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Decision") float Score = -BIG_NUMBER;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Decision") FString Factors;
};

USTRUCT(BlueprintType)
struct FUEGT1NPCSimulationState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Identity") FName NpcId;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Identity") FString DisplayName;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Identity") FName HomeId;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Identity") FName BedId;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Identity") FName JobId;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|State") FName CurrentVenueId;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|State") FName DestinationId;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|State") EUEGT1SimActionType CurrentAction = EUEGT1SimActionType::Idle;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|State") EUEGT1SimActionType PlannedAction = EUEGT1SimActionType::Idle;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|State") FVector WorldLocation = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|State") TArray<FVector> TravelPath;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|State") int32 TravelPathIndex = 0;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|State") float RemainingActionMinutes = 0.0f;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|State") float Money = 0.0f;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|State") FUEGT1NeedState Needs;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|State") FUEGT1DailySchedule Schedule;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Relationships") TArray<FUEGT1HouseholdRelationshipState> HouseholdRelationships;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Motivation") float SavingsGoal = 50.0f;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Motivation") float WorkDrive = 1.0f;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Motivation") float PreferredWorkSessionHours = 4.0f;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Motivation") float CurrentWorkSessionMinutes = 0.0f;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Motivation") float LongestWorkSessionMinutes = 0.0f;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Metrics") float TotalWorkMinutes = 0.0f;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Motivation") FName CurrentWorkVenueId;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Motivation") FString CurrentThought;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Decision") TArray<FUEGT1ActionUtilityScore> UtilityScores;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Decision") FString LatestFailureReason;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Metrics") int32 CompletedSleepActions = 0;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Metrics") int32 CompletedEatActions = 0;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Metrics") int32 CompletedHygieneActions = 0;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Metrics") int32 CompletedSocialActions = 0;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Metrics") int32 CompletedWorkActions = 0;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Metrics") float ConsecutiveIdleMinutes = 0.0f;
	// New skills, relationships, inventory, vehicle, and crime systems can attach scalar state without changing the core planner.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Simulation|Extensions") TMap<FName, float> ExtensionValues;
};

USTRUCT(BlueprintType)
struct FUEGT1PlayerSimulationState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Player") float Money = 40.0f;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Player") FUEGT1NeedState Needs = { 0.86f, 0.82f, 0.88f, 0.80f };
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Player") EUEGT1SimActionType LastAction = EUEGT1SimActionType::Idle;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Player") FName LastVenueId;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Player") FString LastActivityMessage;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Player") int32 CompletedActivities = 0;
};

USTRUCT(BlueprintType)
struct FUEGT1SimulationMetrics
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Metrics") int32 ReplanCount = 0;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Metrics") int32 FailedDestinationCount = 0;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Metrics") int32 AffordabilityFallbackCount = 0;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Metrics") float MoneySpent = 0.0f;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Metrics") float MoneyEarned = 0.0f;
};

USTRUCT(BlueprintType)
struct FUEGT1TownSimulationSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Save") int32 Version = 4;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Save") int32 Seed = 0;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Save") float AbsoluteMinutes = 0.0f;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Save") TArray<FUEGT1TownVenueState> Venues;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Save") TArray<FUEGT1NPCSimulationState> NPCs;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Save") FUEGT1PlayerSimulationState Player;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Simulation|Save") FUEGT1SimulationMetrics Metrics;
};
