#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UEGT1PlayerController.generated.h"

UCLASS()
class UEGT1_API AUEGT1PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	void OpenMenu(bool bInitialMenu = false);
	void CloseMenu();
	void OpenGraphicsMenuForAutomation();
	void OpenLevelMenuForAutomation();
	void TravelToSignalGrove();
	void TravelToTechDemo();
	void ToggleDeveloperMode();
	void ToggleDeveloperFlight();
	bool IsDeveloperModeEnabled() const;
	bool IsDeveloperFlightEnabled() const;
	void RequestQuitFromMenu();
	bool IsMenuOpen() const { return MenuWidget.IsValid(); }
	bool IsWorldMapOpen() const { return bWorldMapOpen; }
	void OpenWorldMapForAutomation();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	void ToggleMenu();
	void ToggleWorldMap();
	void SetWorldMapOpen(bool bOpen);
	void TravelToLevel(FName LevelName);

	TSharedPtr<class SUEGT1Menu> MenuWidget;
	bool bWorldMapOpen = false;
};
