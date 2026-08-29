#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Simulation/UEGT1TownSimulationTypes.h"
#include "UEGT1TownSimulationSettings.generated.h"

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Signal Grove Town Simulation"))
class UEGT1_API UUEGT1TownSimulationSettings final : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UUEGT1TownSimulationSettings();

	static const UUEGT1TownSimulationSettings& Get();
	FUEGT1SimulationTuning MakeTuning() const;

	UPROPERTY(Config, EditAnywhere, Category = "Population", meta = (ClampMin = "100", ClampMax = "100"))
	int32 NPCCount = 100;

	UPROPERTY(Config, EditAnywhere, Category = "Generation")
	int32 TownSeed = 7319;

	UPROPERTY(Config, EditAnywhere, Category = "Simulation")
	FUEGT1SimulationTuning Tuning;

	UPROPERTY(Config, EditAnywhere, Category = "Simulation|Actions")
	TArray<FUEGT1ActionDefinition> ActionDefinitions;

	UPROPERTY(Config, EditAnywhere, Category = "Simulation|Jobs")
	TArray<FUEGT1JobDefinition> JobDefinitions;

	UPROPERTY(Config, EditAnywhere, Category = "Persistence")
	FString DefaultSaveSlot = TEXT("UEGT1TownSimulation");
};
