#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Tickable.h"
#include "Simulation/UEGT1TownSimulationModel.h"
#include "UEGT1TownSimulationSubsystem.generated.h"

class ADirectionalLight;
class AExponentialHeightFog;
class APostProcessVolume;
class ASkyLight;
class AUEGT1TownResident;
class UUEGT1TownDestinationComponent;

UCLASS()
class UEGT1_API UUEGT1TownSimulationSubsystem final : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;

	void RegisterDestination(UUEGT1TownDestinationComponent* Destination);
	void UnregisterDestination(UUEGT1TownDestinationComponent* Destination);
	void StartSimulation(int32 RequestedSeed = 0);

	UFUNCTION(BlueprintCallable, Category = "Town Simulation|Time")
	void AdvanceSimulationMinutes(float Minutes);

	bool CanPlayerPerformActivity(EUEGT1SimActionType Action, FName VenueId, FString& OutReason) const;
	bool PerformPlayerActivity(EUEGT1SimActionType Action, FName VenueId, FString& OutResult);

	UFUNCTION(BlueprintCallable, Category = "Town Simulation|Persistence")
	bool SaveSimulation(const FString& SlotName = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Town Simulation|Persistence")
	bool LoadSimulation(const FString& SlotName = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Town Simulation|Debug")
	void CycleInspectedNPC(int32 Direction = 1);

	UFUNCTION(BlueprintPure, Category = "Town Simulation|Debug")
	FUEGT1NPCSimulationState GetInspectedNPC() const;

	const FUEGT1TownSimulationModel& GetModel() const { return Model; }
	bool GetResidentPresentationLocation(int32 NPCIndex, FVector& OutLocation) const;
	bool IsSimulationRunning() const { return Model.IsInitialized(); }

private:
	void SpawnOrRefreshResidentActors();
	void SyncResidentActorTargets(float BlendDurationSeconds);
	void AdvanceResidentVisuals(float DeltaTime);
	void UpdateDayNightLighting();
	FString ResolveSaveSlot(const FString& RequestedSlot) const;

	TArray<TWeakObjectPtr<UUEGT1TownDestinationComponent>> RegisteredDestinations;
	TArray<TWeakObjectPtr<AUEGT1TownResident>> ResidentActors;
	TWeakObjectPtr<ADirectionalLight> SimulationSun;
	TWeakObjectPtr<ASkyLight> SimulationSkyLight;
	TWeakObjectPtr<AExponentialHeightFog> SimulationFog;
	TWeakObjectPtr<APostProcessVolume> SimulationExposure;
	FUEGT1TownSimulationModel Model;
	float RealTimeAccumulator = 0.0f;
	int32 InspectedNPCIndex = 0;
	bool bLoggedVisualInterpolation = false;
	int32 LastLoggedDayPhase = INDEX_NONE;
	bool bShuttingDown = false;
};
