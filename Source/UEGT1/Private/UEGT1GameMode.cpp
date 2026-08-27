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
#include "World/UEGT1TechDemoEnvironment.h"

namespace
{
	struct FUEGT1RegionCaptureView
	{
		const TCHAR* Name;
		FVector Position;
		FRotator Rotation;
		float HeightAboveSurface;
	};

	const FUEGT1RegionCaptureView RegionCaptureViews[] = {
		{ TEXT("CenterTown"), FVector(0.0f, -1150.0f, 0.0f), FRotator(-4.0f, 90.0f, 0.0f), 115.0f },
		{ TEXT("EastWaterfront"), FVector(6000.0f, 900.0f, 0.0f), FRotator(2.0f, -45.0f, 0.0f), 350.0f },
		{ TEXT("WestFarmland"), FVector(-8500.0f, -2500.0f, 0.0f), FRotator(-5.0f, 180.0f, 0.0f), 300.0f },
		{ TEXT("NorthHighlands"), FVector(0.0f, 8500.0f, 0.0f), FRotator(4.0f, 90.0f, 0.0f), 1100.0f },
		{ TEXT("SouthTropics"), FVector(0.0f, -8500.0f, 0.0f), FRotator(-4.0f, -90.0f, 0.0f), 250.0f }
	};

	struct FUEGT1TechDemoCaptureView
	{
		const TCHAR* Name;
		FVector Position;
		FVector Target;
		float HeightAboveSurface;
	};

	const FUEGT1TechDemoCaptureView TechDemoCaptureViews[] = {
		{ TEXT("ValleyApproach"), FVector(0.0f, -13200.0f, 0.0f), FVector(0.0f, 2500.0f, 500.0f), 900.0f },
		{ TEXT("LakeOverlook"), FVector(-6200.0f, 1000.0f, 0.0f), FVector(0.0f, 3400.0f, 350.0f), 1250.0f },
		{ TEXT("CanopyFlight"), FVector(6500.0f, -2600.0f, 5600.0f), FVector(0.0f, 4000.0f, 450.0f), 0.0f }
	};
}

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
	bIsTechDemoMap = MapName.Contains(TEXT("TechDemo"));

	// World Partition can initialize the first streaming source before a spatial PlayerStart is present.
	// Keep login deterministic by supplying a lightweight, runtime-only start at the authored location.
	const FVector StartLocation = bIsTechDemoMap ? AUEGT1TechDemoEnvironment::GetRecommendedPlayerStart() : UEGT1WorldLayout::GetPlayerStartLocation();
	const FTransform StartTransform(FRotator(0.0f, 90.0f, 0.0f), StartLocation);
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
	for (TActorIterator<AUEGT1WorldDirector> Iterator(GetWorld()); !bIsTechDemoMap && Iterator; ++Iterator)
	{
		bHasWorldDirector = true;
		break;
	}
	if (!bIsTechDemoMap && !bHasWorldDirector)
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
	if (FParse::Value(FCommandLine::Get(), TEXT("UEGT1LevelMenuCapture="), AutomatedLevelMenuCapturePath))
	{
		AutomatedLevelMenuCapturePath = FPaths::ConvertRelativePathToFull(AutomatedLevelMenuCapturePath);
		GetWorldTimerManager().SetTimer(LevelMenuCaptureTimerHandle, this, &AUEGT1GameMode::CaptureAutomatedLevelMenuFrame, 4.0f, false);
		UE_LOG(LogUEGT1, Display, TEXT("Automated level-selection menu capture scheduled: %s"), *AutomatedLevelMenuCapturePath);
	}
	if (!bIsTechDemoMap && FParse::Param(FCommandLine::Get(), TEXT("UEGT1SmokeSelectTechDemo")))
	{
		GetWorldTimerManager().SetTimer(LevelTravelTimerHandle, this, &AUEGT1GameMode::SelectAutomatedTechDemo, 3.0f, false);
		UE_LOG(LogUEGT1, Display, TEXT("Automated menu travel to Lumen Wilds scheduled."));
	}
	if (FParse::Value(FCommandLine::Get(), TEXT("UEGT1RegionCaptureFolder="), AutomatedRegionCaptureFolder))
	{
		AutomatedRegionCaptureFolder = FPaths::ConvertRelativePathToFull(AutomatedRegionCaptureFolder);
		IFileManager::Get().MakeDirectory(*AutomatedRegionCaptureFolder, true);
		AutomatedRegionCaptureIndex = 0;
		GetWorldTimerManager().SetTimer(RegionCaptureTimerHandle, this, &AUEGT1GameMode::PositionAutomatedRegionView, 4.0f, false);
		UE_LOG(LogUEGT1, Display, TEXT("Automated regional capture sequence scheduled: %s"), *AutomatedRegionCaptureFolder);
	}
	if (bIsTechDemoMap && FParse::Value(FCommandLine::Get(), TEXT("UEGT1TechDemoCaptureFolder="), AutomatedTechDemoCaptureFolder))
	{
		AutomatedTechDemoCaptureFolder = FPaths::ConvertRelativePathToFull(AutomatedTechDemoCaptureFolder);
		IFileManager::Get().MakeDirectory(*AutomatedTechDemoCaptureFolder, true);
		AutomatedTechDemoCaptureIndex = 0;
		GetWorldTimerManager().SetTimer(TechDemoCaptureTimerHandle, this, &AUEGT1GameMode::PositionAutomatedTechDemoView, 5.0f, false);
		UE_LOG(LogUEGT1, Display, TEXT("Automated Lumen Wilds capture sequence scheduled: %s"), *AutomatedTechDemoCaptureFolder);
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

void AUEGT1GameMode::CaptureAutomatedLevelMenuFrame()
{
	if (AUEGT1PlayerController* PlayerController = Cast<AUEGT1PlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		PlayerController->OpenLevelMenuForAutomation();
	}
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(AutomatedLevelMenuCapturePath), true);
	FScreenshotRequest::RequestScreenshot(AutomatedLevelMenuCapturePath, true, false);
	UE_LOG(LogUEGT1, Display, TEXT("Automated level-selection menu screenshot requested: %s"), *AutomatedLevelMenuCapturePath);
	GetWorldTimerManager().SetTimer(ExitTimerHandle, this, &AUEGT1GameMode::FinishAutomatedMenuSmokeRun, 3.0f, false);
}

void AUEGT1GameMode::SelectAutomatedTechDemo()
{
	if (AUEGT1PlayerController* PlayerController = Cast<AUEGT1PlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		PlayerController->OpenLevelMenuForAutomation();
		PlayerController->TravelToTechDemo();
		return;
	}
	UE_LOG(LogUEGT1, Error, TEXT("Automated menu travel could not find the player controller."));
	FGenericPlatformMisc::RequestExit(true);
}

void AUEGT1GameMode::PositionAutomatedRegionView()
{
	if (AutomatedRegionCaptureIndex >= static_cast<int32>(UE_ARRAY_COUNT(RegionCaptureViews)))
	{
		FinishAutomatedSmokeRun();
		return;
	}
	const FUEGT1RegionCaptureView& View = RegionCaptureViews[AutomatedRegionCaptureIndex];
	const FUEGT1RegionSample Sample = UEGT1WorldLayout::SampleRegion(View.Position);
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (APawn* Pawn = PlayerController->GetPawn())
		{
			FVector PawnPosition = View.Position;
			PawnPosition.Z = Sample.SurfaceHeight + View.HeightAboveSurface;
			Pawn->SetActorLocation(PawnPosition, false, nullptr, ETeleportType::TeleportPhysics);
		}
		PlayerController->SetControlRotation(View.Rotation);
	}
	UE_LOG(LogUEGT1, Display, TEXT("Automated regional view positioned: View=%s Dominant=%s Surface=%.0f Water=%.0f Blend=%s"),
		View.Name, LexToString(Sample.GetDominantBiome()), Sample.SurfaceHeight, Sample.WaterDepth, *Sample.Biomes.ToCompactString());
	GetWorldTimerManager().SetTimer(RegionCaptureTimerHandle, this, &AUEGT1GameMode::CaptureAutomatedRegionFrame, 0.75f, false);
}

void AUEGT1GameMode::CaptureAutomatedRegionFrame()
{
	const FUEGT1RegionCaptureView& View = RegionCaptureViews[AutomatedRegionCaptureIndex];
	const FString CapturePath = FPaths::Combine(AutomatedRegionCaptureFolder, FString::Printf(TEXT("%02d-%s.png"), AutomatedRegionCaptureIndex + 1, View.Name));
	FScreenshotRequest::RequestScreenshot(CapturePath, false, false);
	UE_LOG(LogUEGT1, Display, TEXT("Automated regional screenshot requested: View=%s Path=%s"), View.Name, *CapturePath);
	++AutomatedRegionCaptureIndex;
	if (AutomatedRegionCaptureIndex < UE_ARRAY_COUNT(RegionCaptureViews))
	{
		GetWorldTimerManager().SetTimer(RegionCaptureTimerHandle, this, &AUEGT1GameMode::PositionAutomatedRegionView, 1.25f, false);
	}
	else
	{
		GetWorldTimerManager().SetTimer(ExitTimerHandle, this, &AUEGT1GameMode::FinishAutomatedSmokeRun, 2.0f, false);
	}
}

void AUEGT1GameMode::PositionAutomatedTechDemoView()
{
	if (AutomatedTechDemoCaptureIndex >= static_cast<int32>(UE_ARRAY_COUNT(TechDemoCaptureViews)))
	{
		FinishAutomatedSmokeRun();
		return;
	}
	const FUEGT1TechDemoCaptureView& View = TechDemoCaptureViews[AutomatedTechDemoCaptureIndex];
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (APawn* Pawn = PlayerController->GetPawn())
		{
			FVector Position = View.Position;
			if (AutomatedTechDemoCaptureIndex < 2)
			{
				Position.Z = AUEGT1TechDemoEnvironment::SampleTerrainHeight(FVector2D(Position.X, Position.Y)) + View.HeightAboveSurface;
			}
			Pawn->SetActorLocation(Position, false, nullptr, ETeleportType::TeleportPhysics);
			PlayerController->SetControlRotation((View.Target - Position).Rotation());
		}
	}
	UE_LOG(LogUEGT1, Display, TEXT("Automated Lumen Wilds view positioned: View=%s"), View.Name);
	GetWorldTimerManager().SetTimer(TechDemoCaptureTimerHandle, this, &AUEGT1GameMode::CaptureAutomatedTechDemoFrame, 1.0f, false);
}

void AUEGT1GameMode::CaptureAutomatedTechDemoFrame()
{
	const FUEGT1TechDemoCaptureView& View = TechDemoCaptureViews[AutomatedTechDemoCaptureIndex];
	const FString CapturePath = FPaths::Combine(AutomatedTechDemoCaptureFolder,
		FString::Printf(TEXT("%02d-%s.png"), AutomatedTechDemoCaptureIndex + 1, View.Name));
	FScreenshotRequest::RequestScreenshot(CapturePath, false, false);
	UE_LOG(LogUEGT1, Display, TEXT("Automated Lumen Wilds screenshot requested: View=%s Path=%s"), View.Name, *CapturePath);
	++AutomatedTechDemoCaptureIndex;
	if (AutomatedTechDemoCaptureIndex < UE_ARRAY_COUNT(TechDemoCaptureViews))
	{
		GetWorldTimerManager().SetTimer(TechDemoCaptureTimerHandle, this, &AUEGT1GameMode::PositionAutomatedTechDemoView, 1.4f, false);
	}
	else
	{
		GetWorldTimerManager().SetTimer(ExitTimerHandle, this, &AUEGT1GameMode::FinishAutomatedSmokeRun, 2.0f, false);
	}
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
