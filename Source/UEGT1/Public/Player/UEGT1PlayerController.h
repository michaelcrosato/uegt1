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
	void RequestQuitFromMenu();
	bool IsMenuOpen() const { return MenuWidget.IsValid(); }

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	void ToggleMenu();

	TSharedPtr<class SUEGT1Menu> MenuWidget;
};
