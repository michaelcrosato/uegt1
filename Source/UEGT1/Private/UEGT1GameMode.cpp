#include "UEGT1GameMode.h"

#include "Components/PrimitiveComponent.h"
#include "EngineUtils.h"
#include "Gameplay/UEGT1MilestoneGameState.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformMisc.h"
#include "GameFramework/PlayerStart.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Materials/MaterialInterface.h"
#include "Player/UEGT1ExplorerCharacter.h"
#include "Player/UEGT1PlayerController.h"
#include "Settings/UEGT1GameUserSettings.h"
#include "TimerManager.h"
#include "UI/UEGT1HUD.h"
#include "UEGT1LogChannels.h"
#include "UnrealClient.h"
#include "World/UEGT1WorldDirector.h"
#include "World/UEGT1WorldLayout.h"
#include "World/UEGT1BiomeTile.h"

AUEGT1GameMode::AUEGT1GameMode()
{
	DefaultPawnClass = AUEGT1ExplorerCharacter::StaticClass();
	PlayerControllerClass = AUEGT1PlayerController::StaticClass();
	GameStateClass = AUEGT1MilestoneGameState::StaticClass();
	HUDClass = AUEGT1HUD::StaticClass();
}

void AUEGT1GameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// World Partition can initialize the first streaming source before a spatial PlayerStart is present.
	// Keep login deterministic by supplying a lightweight, runtime-only start at the authored location.
	const FTransform StartTransform(FRotator(0.0f, 90.0f, 0.0f), UEGT1WorldLayout::PlayerStartLocation);
	RuntimePlayerStart = GetWorld()->SpawnActor<APlayerStart>(APlayerStart::StaticClass(), StartTransform.GetLocation(), StartTransform.Rotator());
	UE_LOG(LogUEGT1, Display, TEXT("Runtime PlayerStart ready: Location=%s Rotation=%s"),
		*StartTransform.GetLocation().ToCompactString(), *StartTransform.Rotator().ToCompactString());
}

AActor* AUEGT1GameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	return RuntimePlayerStart ? RuntimePlayerStart.Get() : Super::ChoosePlayerStart_Implementation(Player);
}

void AUEGT1GameMode::StartPlay()
{
	bool bHasWorldDirector = false;
	for (TActorIterator<AUEGT1WorldDirector> Iterator(GetWorld()); Iterator; ++Iterator)
	{
		bHasWorldDirector = true;
		break;
	}
	if (!bHasWorldDirector)
	{
		GetWorld()->SpawnActor<AUEGT1WorldDirector>(AUEGT1WorldDirector::StaticClass());
	}

	Super::StartPlay();

	if (FParse::Param(FCommandLine::Get(), TEXT("UEGT1SmokeResetSettings")))
	{
		if (UUEGT1GameUserSettings* Settings = UUEGT1GameUserSettings::Get())
		{
			Settings->SetRecommendedDefaults();
			Settings->ApplySettings(false);
			Settings->ConfirmVideoMode();
			Settings->SaveSettings();
			UE_LOG(LogUEGT1, Display, TEXT("Automated smoke restored recommended settings before capture."));
		}
	}

	if (FParse::Value(FCommandLine::Get(), TEXT("UEGT1SmokeCapture="), AutomatedCapturePath))
	{
		AutomatedCapturePath = FPaths::ConvertRelativePathToFull(AutomatedCapturePath);
		GetWorldTimerManager().SetTimer(CaptureTimerHandle, this, &AUEGT1GameMode::CaptureAutomatedSmokeFrame, 4.0f, false);
		UE_LOG(LogUEGT1, Display, TEXT("Automated gameplay capture scheduled: %s"), *AutomatedCapturePath);
	}
	if (FParse::Value(FCommandLine::Get(), TEXT("UEGT1SmokeMenuCapture="), AutomatedMenuCapturePath))
	{
		AutomatedMenuCapturePath = FPaths::ConvertRelativePathToFull(AutomatedMenuCapturePath);
		GetWorldTimerManager().SetTimer(MenuCaptureTimerHandle, this, &AUEGT1GameMode::CaptureAutomatedMenuFrame, 4.0f, false);
		UE_LOG(LogUEGT1, Display, TEXT("Automated menu capture scheduled: %s"), *AutomatedMenuCapturePath);
	}
}

void AUEGT1GameMode::CaptureAutomatedSmokeFrame()
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		FVector ViewLocation;
		FRotator ViewRotation;
		PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
		UE_LOG(LogUEGT1, Display, TEXT("Automated view: Location=%s Rotation=%s"),
			*ViewLocation.ToCompactString(), *ViewRotation.ToCompactString());

		for (const float PitchOffset : { -15.0f, -35.0f, -60.0f })
		{
			const FVector End = ViewLocation + (ViewRotation + FRotator(PitchOffset, 0.0f, 0.0f)).Vector() * 5000.0f;
			FHitResult Hit;
			if (GetWorld()->LineTraceSingleByChannel(Hit, ViewLocation, End, ECC_Visibility))
			{
				const UPrimitiveComponent* Component = Hit.GetComponent();
				const UMaterialInterface* Material = Component ? Component->GetMaterial(0) : nullptr;
				const AUEGT1BiomeTile* Tile = Cast<AUEGT1BiomeTile>(Hit.GetActor());
				UE_LOG(LogUEGT1, Display, TEXT("Automated view trace: Pitch=%.0f Actor=%s ActorLocation=%s Tile=%s Component=%s Material=%s Impact=%s"),
					PitchOffset, *GetNameSafe(Hit.GetActor()), Hit.GetActor() ? *Hit.GetActor()->GetActorLocation().ToCompactString() : TEXT("None"),
					Tile ? *Tile->GetTileCoordinate().ToString() : TEXT("None"), *GetNameSafe(Component), *GetNameSafe(Material), *Hit.ImpactPoint.ToCompactString());
			}
		}
	}

	if (FParse::Param(FCommandLine::Get(), TEXT("UEGT1SmokeComplete")))
	{
		if (AUEGT1MilestoneGameState* MilestoneState = GetGameState<AUEGT1MilestoneGameState>())
		{
			for (const FName WaystoneId : UEGT1WorldLayout::GetWaystoneIds())
			{
				MilestoneState->ActivateWaystone(WaystoneId);
			}
		}
	}

	IFileManager::Get().MakeDirectory(*FPaths::GetPath(AutomatedCapturePath), true);
	FScreenshotRequest::RequestScreenshot(AutomatedCapturePath, false, false);
	UE_LOG(LogUEGT1, Display, TEXT("Automated gameplay screenshot requested: %s"), *AutomatedCapturePath);
	GetWorldTimerManager().SetTimer(ExitTimerHandle, this, &AUEGT1GameMode::FinishAutomatedSmokeRun, 3.0f, false);
}

void AUEGT1GameMode::FinishAutomatedSmokeRun()
{
	UE_LOG(LogUEGT1, Display, TEXT("Automated gameplay smoke completed."));
	FGenericPlatformMisc::RequestExit(false);
}

void AUEGT1GameMode::CaptureAutomatedMenuFrame()
{
	if (AUEGT1PlayerController* PlayerController = Cast<AUEGT1PlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		PlayerController->OpenGraphicsMenuForAutomation();
	}
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(AutomatedMenuCapturePath), true);
	FScreenshotRequest::RequestScreenshot(AutomatedMenuCapturePath, true, false);
	UE_LOG(LogUEGT1, Display, TEXT("Automated graphics menu screenshot requested: %s"), *AutomatedMenuCapturePath);
	GetWorldTimerManager().SetTimer(ExitTimerHandle, this, &AUEGT1GameMode::FinishAutomatedMenuSmokeRun, 3.0f, false);
}

void AUEGT1GameMode::FinishAutomatedMenuSmokeRun()
{
	if (UUEGT1GameUserSettings* Settings = UUEGT1GameUserSettings::Get())
	{
		Settings->SetRecommendedDefaults();
		Settings->ApplySettings(false);
		Settings->ConfirmVideoMode();
		Settings->SaveSettings();
		UE_LOG(LogUEGT1, Display, TEXT("Automated menu smoke restored recommended settings."));
	}
	if (AUEGT1PlayerController* PlayerController = Cast<AUEGT1PlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		PlayerController->RequestQuitFromMenu();
		return;
	}
	UE_LOG(LogUEGT1, Error, TEXT("Automated menu smoke could not find the player controller."));
	FGenericPlatformMisc::RequestExit(true);
}
