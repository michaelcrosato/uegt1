#include "Player/UEGT1PlayerController.h"

#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Framework/Application/SlateApplication.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Settings/UEGT1GameUserSettings.h"
#include "UI/SUEGT1Menu.h"
#include "UEGT1LogChannels.h"
#include "TimerManager.h"

void AUEGT1PlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UUEGT1GameUserSettings* Settings = UUEGT1GameUserSettings::Get())
	{
		Settings->ApplyNonResolutionSettings();
	}

	bShowMouseCursor = false;
	FInputModeGameOnly InputMode;
	InputMode.SetConsumeCaptureMouseDown(true);
	SetInputMode(InputMode);

	if (!FApp::IsUnattended())
	{
		GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]() { OpenMenu(true); }));
	}
}

void AUEGT1PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (InputComponent)
	{
		InputComponent->BindAction(TEXT("PauseMenu"), IE_Pressed, this, &AUEGT1PlayerController::ToggleMenu);
	}
}

void AUEGT1PlayerController::ToggleMenu()
{
	if (IsMenuOpen())
	{
		CloseMenu();
	}
	else
	{
		OpenMenu(false);
	}
}

void AUEGT1PlayerController::OpenMenu(bool bInitialMenu)
{
	if (IsMenuOpen() || !GEngine || !GEngine->GameViewport)
	{
		return;
	}

	SAssignNew(MenuWidget, SUEGT1Menu)
		.OwnerController(this)
		.IsInitialMenu(bInitialMenu);
	GEngine->GameViewport->AddViewportWidgetContent(MenuWidget.ToSharedRef(), 1000);

	if (!FApp::IsUnattended())
	{
		SetPause(true);
	}
	bShowMouseCursor = true;
	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(MenuWidget);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
	MenuWidget->SetInitialFocus();
	UE_LOG(LogUEGT1, Display, TEXT("Game menu opened: Initial=%s"), bInitialMenu ? TEXT("true") : TEXT("false"));
}

void AUEGT1PlayerController::CloseMenu()
{
	if (!IsMenuOpen() || !GEngine || !GEngine->GameViewport)
	{
		return;
	}

	GEngine->GameViewport->RemoveViewportWidgetContent(MenuWidget.ToSharedRef());
	MenuWidget.Reset();
	SetPause(false);
	bShowMouseCursor = false;
	FInputModeGameOnly InputMode;
	InputMode.SetConsumeCaptureMouseDown(true);
	SetInputMode(InputMode);
	FSlateApplication::Get().SetAllUserFocusToGameViewport();
	UE_LOG(LogUEGT1, Display, TEXT("Game menu closed; play resumed."));
}

void AUEGT1PlayerController::OpenGraphicsMenuForAutomation()
{
	OpenMenu(false);
	if (MenuWidget)
	{
		if (UUEGT1GameUserSettings* Settings = UUEGT1GameUserSettings::Get())
		{
			Settings->SetRecommendedDefaults();
			Settings->SetAllOptionalFeaturesEnabled(false);
		}
		MenuWidget->ShowGraphicsPage();
	}
}

void AUEGT1PlayerController::RequestQuitFromMenu()
{
	UE_LOG(LogUEGT1, Display, TEXT("Menu quit requested; exiting to desktop."));
	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
}
