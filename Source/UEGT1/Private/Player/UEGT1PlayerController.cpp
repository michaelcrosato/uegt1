#include "Player/UEGT1PlayerController.h"

#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Development/UEGT1DeveloperModeSubsystem.h"
#include "Framework/Application/SlateApplication.h"
#include "Gameplay/UEGT1SessionSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Player/UEGT1ExplorerCharacter.h"
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

	const UUEGT1SessionSubsystem* Session = GetGameInstance()->GetSubsystem<UUEGT1SessionSubsystem>();
	if (!FApp::IsUnattended() && (!Session || !Session->HasSelectedInitialLevel()))
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
		InputComponent->BindAction(TEXT("ToggleWorldMap"), IE_Pressed, this, &AUEGT1PlayerController::ToggleWorldMap);
	}
}

void AUEGT1PlayerController::ToggleMenu()
{
	if (bWorldMapOpen)
	{
		SetWorldMapOpen(false);
		return;
	}
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
	SetWorldMapOpen(false);

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

void AUEGT1PlayerController::ToggleWorldMap()
{
	if (IsMenuOpen() || UGameplayStatics::GetCurrentLevelName(GetWorld(), true).Contains(TEXT("TechDemo")))
	{
		return;
	}
	SetWorldMapOpen(!bWorldMapOpen);
}

void AUEGT1PlayerController::SetWorldMapOpen(bool bOpen)
{
	if (bWorldMapOpen == bOpen)
	{
		return;
	}
	bWorldMapOpen = bOpen;
	SetIgnoreMoveInput(bOpen);
	SetIgnoreLookInput(bOpen);
	UE_LOG(LogUEGT1, Display, TEXT("World map %s."), bWorldMapOpen ? TEXT("opened") : TEXT("closed"));
}

void AUEGT1PlayerController::OpenWorldMapForAutomation()
{
	if (!bWorldMapOpen)
	{
		SetWorldMapOpen(true);
	}
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

void AUEGT1PlayerController::OpenLevelMenuForAutomation()
{
	OpenMenu(true);
}

void AUEGT1PlayerController::TravelToSignalGrove()
{
	TravelToLevel(TEXT("Main"));
}

void AUEGT1PlayerController::TravelToTechDemo()
{
	TravelToLevel(TEXT("TechDemo"));
}

void AUEGT1PlayerController::TravelToLevel(FName LevelName)
{
	if (LevelName != TEXT("Main") && LevelName != TEXT("TechDemo"))
	{
		UE_LOG(LogUEGT1, Error, TEXT("Rejected unknown level selection: %s"), *LevelName.ToString());
		return;
	}
	if (UUEGT1SessionSubsystem* Session = GetGameInstance()->GetSubsystem<UUEGT1SessionSubsystem>())
	{
		Session->MarkInitialLevelSelected();
	}
	if (IsMenuOpen())
	{
		CloseMenu();
	}
	SetWorldMapOpen(false);
	UE_LOG(LogUEGT1, Display, TEXT("Level selected from menu: %s"), *LevelName.ToString());
	UGameplayStatics::OpenLevel(this, LevelName);
}

void AUEGT1PlayerController::ToggleDeveloperMode()
{
	if (UUEGT1DeveloperModeSubsystem* DeveloperMode = GetGameInstance()->GetSubsystem<UUEGT1DeveloperModeSubsystem>())
	{
		DeveloperMode->SetEnabled(!DeveloperMode->IsEnabled());
		if (AUEGT1ExplorerCharacter* Explorer = Cast<AUEGT1ExplorerCharacter>(GetPawn()))
		{
			Explorer->RefreshDeveloperMode();
		}
	}
}

void AUEGT1PlayerController::ToggleDeveloperFlight()
{
	if (UUEGT1DeveloperModeSubsystem* DeveloperMode = GetGameInstance()->GetSubsystem<UUEGT1DeveloperModeSubsystem>())
	{
		DeveloperMode->SetFlightEnabled(!DeveloperMode->IsFlightEnabled());
		if (AUEGT1ExplorerCharacter* Explorer = Cast<AUEGT1ExplorerCharacter>(GetPawn()))
		{
			Explorer->RefreshDeveloperMode();
		}
	}
}

bool AUEGT1PlayerController::IsDeveloperModeEnabled() const
{
	const UUEGT1DeveloperModeSubsystem* DeveloperMode = GetGameInstance()->GetSubsystem<UUEGT1DeveloperModeSubsystem>();
	return DeveloperMode && DeveloperMode->IsEnabled();
}

bool AUEGT1PlayerController::IsDeveloperFlightEnabled() const
{
	const UUEGT1DeveloperModeSubsystem* DeveloperMode = GetGameInstance()->GetSubsystem<UUEGT1DeveloperModeSubsystem>();
	return DeveloperMode && DeveloperMode->IsFlightEnabled();
}

void AUEGT1PlayerController::RequestQuitFromMenu()
{
	UE_LOG(LogUEGT1, Display, TEXT("Menu quit requested; exiting to desktop."));
	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
}
