#pragma once

#include "CoreMinimal.h"
#include "Simulation/UEGT1TownSimulationTypes.h"

class UEGT1_API FUEGT1TownSimulationModel
{
public:
	void Initialize(int32 InSeed, int32 NPCCount, const TArray<FUEGT1TownVenueState>& InVenues,
		const FUEGT1SimulationTuning& InTuning);
	bool Restore(const FUEGT1TownSimulationSnapshot& Snapshot, const FUEGT1SimulationTuning& InTuning);
	FUEGT1TownSimulationSnapshot MakeSnapshot() const;
	void AdvanceMinutes(float Minutes);

	bool IsInitialized() const { return bInitialized; }
	int32 GetSeed() const { return Seed; }
	float GetAbsoluteMinutes() const { return AbsoluteMinutes; }
	int32 GetDayIndex() const { return FMath::FloorToInt(AbsoluteMinutes / 1440.0f); }
	float GetHourOfDay() const { return FMath::Fmod(AbsoluteMinutes, 1440.0f) / 60.0f; }
	FDateTime GetCalendarDateTime() const;
	const TArray<FUEGT1TownVenueState>& GetVenues() const { return Venues; }
	const TArray<FUEGT1NPCSimulationState>& GetNPCs() const { return NPCs; }
	const FUEGT1SimulationMetrics& GetMetrics() const { return Metrics; }
	const FUEGT1PlayerSimulationState& GetPlayer() const { return Player; }
	int32 GetBedCount() const;
	bool HasCompleteBedAssignments() const;
	bool HasCompleteHouseholdRelationships() const;
	FUEGT1NPCSimulationState* FindNPC(FName NpcId);
	FUEGT1TownVenueState* FindVenue(FName VenueId);
	FUEGT1ActionUtilityScore EvaluateBestAction(int32 NPCIndex, bool bCommitUtilityScores = false);
	bool CanPlayerPerformActivity(EUEGT1SimActionType Action, FName VenueId, FString& OutReason) const;
	bool PerformPlayerActivity(EUEGT1SimActionType Action, FName VenueId, FString& OutResult);

private:
	void AdvanceStep(float Minutes);
	void DecayNeeds(FUEGT1NPCSimulationState& NPC, float Minutes) const;
	void AdvanceTravel(FUEGT1NPCSimulationState& NPC, float Minutes);
	void CompleteAction(FUEGT1NPCSimulationState& NPC);
	void PlanNPC(FUEGT1NPCSimulationState& NPC);
	void UpdateThought(FUEGT1NPCSimulationState& NPC, const FUEGT1TownVenueState* Venue = nullptr) const;
	void FailAndReplan(FUEGT1NPCSimulationState& NPC, const FString& Reason);
	void Reserve(FUEGT1TownVenueState& Venue, FName NpcId);
	void ReleaseReservation(FName VenueId, FName NpcId);
	const FUEGT1TownVenueState* FindVenue(FName VenueId) const;
	const FUEGT1ActionDefinition* FindActionDefinition(EUEGT1SimActionType Action, EUEGT1TownVenueType VenueType) const;
	FUEGT1ActionUtilityScore EvaluateDefinition(const FUEGT1NPCSimulationState& NPC,
		const FUEGT1ActionDefinition& Definition, bool& bOutUnaffordable) const;
	float CalculateTravelDistance(const FUEGT1NPCSimulationState& NPC, const FUEGT1TownVenueState& Destination,
		EUEGT1SimActionType Action) const;
	TArray<FVector> BuildTravelPath(const FUEGT1NPCSimulationState& NPC, const FUEGT1TownVenueState& Destination,
		EUEGT1SimActionType Action) const;
	FVector GetActivityLocation(const FUEGT1NPCSimulationState& NPC, const FUEGT1TownVenueState& Destination,
		EUEGT1SimActionType Action) const;

	int32 Seed = 0;
	float AbsoluteMinutes = 0.0f;
	FUEGT1SimulationTuning Tuning;
	TArray<FUEGT1TownVenueState> Venues;
	TArray<FUEGT1NPCSimulationState> NPCs;
	FUEGT1SimulationMetrics Metrics;
	FUEGT1PlayerSimulationState Player;
	bool bInitialized = false;
};

namespace UEGT1TownSimulation
{
	UEGT1_API void ApplyActionOutcome(FUEGT1NPCSimulationState& NPC, const FUEGT1ActionDefinition& Definition,
		FUEGT1SimulationMetrics& Metrics);
}
