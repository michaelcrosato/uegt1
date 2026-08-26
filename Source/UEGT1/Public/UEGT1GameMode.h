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
	void CaptureAutomatedMenuFrame();
	void PositionAutomatedRegionView();
	void CaptureAutomatedRegionFrame();
	void FinishAutomatedSmokeRun();
	void FinishAutomatedMenuSmokeRun();

	FString AutomatedCapturePath;
	FString AutomatedMenuCapturePath;
	FString AutomatedRegionCaptureFolder;
	int32 AutomatedRegionCaptureIndex = 0;
	FTimerHandle CaptureTimerHandle;
	FTimerHandle MenuCaptureTimerHandle;
	FTimerHandle RegionCaptureTimerHandle;
	FTimerHandle ExitTimerHandle;

	UPROPERTY(Transient)
	TObjectPtr<AActor> RuntimePlayerStart;
};
