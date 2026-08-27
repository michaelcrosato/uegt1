#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UEGT1DeveloperModeSubsystem.generated.h"

UCLASS()
class UEGT1_API UUEGT1DeveloperModeSubsystem final : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintPure, Category = "UEGT1|Developer Mode")
	bool IsEnabled() const { return bEnabled; }

	UFUNCTION(BlueprintPure, Category = "UEGT1|Developer Mode")
	bool IsFlightEnabled() const { return bEnabled && bFlightEnabled; }

	UFUNCTION(BlueprintCallable, Category = "UEGT1|Developer Mode")
	void SetEnabled(bool bInEnabled);

	UFUNCTION(BlueprintCallable, Category = "UEGT1|Developer Mode")
	void SetFlightEnabled(bool bInFlightEnabled);

private:
	bool bEnabled = false;
	bool bFlightEnabled = false;
};
