#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "UEGT1MilestoneGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FUEGT1ObjectiveProgress, int32, ActivatedCount, int32, TotalCount, bool, bComplete);

UCLASS()
class UEGT1_API AUEGT1MilestoneGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "UEGT1|Objective")
	void RegisterWaystone(FName WaystoneId);

	UFUNCTION(BlueprintCallable, Category = "UEGT1|Objective")
	bool ActivateWaystone(FName WaystoneId);

	UFUNCTION(BlueprintPure, Category = "UEGT1|Objective")
	int32 GetActivatedCount() const { return ActivatedWaystones.Num(); }

	UFUNCTION(BlueprintPure, Category = "UEGT1|Objective")
	int32 GetTotalCount() const { return RegisteredWaystones.Num(); }

	UFUNCTION(BlueprintPure, Category = "UEGT1|Objective")
	bool IsMilestoneComplete() const { return RegisteredWaystones.Num() > 0 && ActivatedWaystones.Num() == RegisteredWaystones.Num(); }

	UPROPERTY(BlueprintAssignable, Category = "UEGT1|Objective")
	FUEGT1ObjectiveProgress OnObjectiveProgress;

private:
	void BroadcastProgress();

	UPROPERTY()
	TSet<FName> RegisteredWaystones;

	UPROPERTY()
	TSet<FName> ActivatedWaystones;
};
