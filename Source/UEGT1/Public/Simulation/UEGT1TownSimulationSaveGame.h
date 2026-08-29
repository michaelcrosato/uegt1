#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Simulation/UEGT1TownSimulationTypes.h"
#include "UEGT1TownSimulationSaveGame.generated.h"

UCLASS()
class UEGT1_API UUEGT1TownSimulationSaveGame final : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(SaveGame)
	FUEGT1TownSimulationSnapshot Snapshot;
};
