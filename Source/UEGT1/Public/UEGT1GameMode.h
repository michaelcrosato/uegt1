#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "UEGT1GameMode.generated.h"

UCLASS()
class UEGT1_API AUEGT1GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AUEGT1GameMode();
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void StartPlay() override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

private:
	void CaptureAutomatedSmokeFrame();
	void FinishAutomatedSmokeRun();

	FString AutomatedCapturePath;
	FTimerHandle CaptureTimerHandle;
	FTimerHandle ExitTimerHandle;

	UPROPERTY(Transient)
	TObjectPtr<AActor> RuntimePlayerStart;
};
