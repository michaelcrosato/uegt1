#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UEGT1PlayerController.generated.h"

UCLASS()
class UEGT1_API AUEGT1PlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
};
