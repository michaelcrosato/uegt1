#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UEGT1SessionSubsystem.generated.h"

UCLASS()
class UEGT1_API UUEGT1SessionSubsystem final : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	bool HasSelectedInitialLevel() const { return bHasSelectedInitialLevel; }
	void MarkInitialLevelSelected() { bHasSelectedInitialLevel = true; }

private:
	bool bHasSelectedInitialLevel = false;
};
