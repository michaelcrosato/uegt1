#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "Engine/Level.h"
#include "Engine/World.h"
#include "GameFramework/InputSettings.h"
#include "HAL/IConsoleManager.h"
#include "Gameplay/UEGT1MilestoneGameState.h"
#include "Gameplay/UEGT1Waystone.h"
#include "Interaction/UEGT1Interactable.h"
#include "Misc/ConfigCacheIni.h"
#include "Player/UEGT1ExplorerCharacter.h"
#include "Player/UEGT1PlayerController.h"
#include "Settings/UEGT1GameUserSettings.h"
#include "UEGT1GameMode.h"
#include "UI/UEGT1HUD.h"
#include "World/UEGT1WorldLayout.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUEGT1ProjectConfigurationTest,
	"UEGT1.Smoke.ProjectConfiguration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUEGT1ProjectConfigurationTest::RunTest(const FString& Parameters)
{
	FString DefaultRHI;
	const bool bFoundRHI = GConfig->GetString(
		TEXT("/Script/WindowsTargetPlatform.WindowsTargetSettings"),
		TEXT("DefaultGraphicsRHI"),
		DefaultRHI,
		GEngineIni);

	TestTrue(TEXT("The Windows default RHI is configured"), bFoundRHI);
	TestEqual(TEXT("DirectX 12 is the Windows default RHI"), DefaultRHI, FString(TEXT("DefaultGraphicsRHI_DX12")));

	const UInputSettings* InputSettings = GetDefault<UInputSettings>();
	for (const FName Action : { FName(TEXT("Jump")), FName(TEXT("Sprint")), FName(TEXT("Interact")), FName(TEXT("ToggleDiagnostics")), FName(TEXT("PauseMenu")) })
	{
		TArray<FInputActionKeyMapping> Mappings;
		InputSettings->GetActionMappingByName(Action, Mappings);
		TestTrue(*FString::Printf(TEXT("%s has an input mapping"), *Action.ToString()), !Mappings.IsEmpty());
	}

	for (const FName Axis : { FName(TEXT("MoveForward")), FName(TEXT("MoveRight")), FName(TEXT("Turn")), FName(TEXT("LookUp")) })
	{
		TArray<FInputAxisKeyMapping> Mappings;
		InputSettings->GetAxisMappingByName(Axis, Mappings);
		TestTrue(*FString::Printf(TEXT("%s has an input mapping"), *Axis.ToString()), !Mappings.IsEmpty());
	}
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUEGT1GraphicsSettingsTest,
	"UEGT1.Smoke.GraphicsSettings",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUEGT1GraphicsSettingsTest::RunTest(const FString& Parameters)
{
	UUEGT1GameUserSettings* Settings = UUEGT1GameUserSettings::Get();
	TestNotNull(TEXT("The engine uses Signal Grove game user settings"), Settings);
	if (!Settings)
	{
		return false;
	}

	Settings->SetRecommendedDefaults();
	TestEqual(TEXT("Recommended resolution targets 1080p"), Settings->GetScreenResolution(), FIntPoint(1920, 1080));
	TestEqual(TEXT("Recommended view distance targets High"), Settings->GetViewDistanceQuality(), 2);
	TestEqual(TEXT("Recommended textures target Epic"), Settings->GetTextureQuality(), 3);
	TestTrue(TEXT("Recommended resolution scale targets 100 percent"), FMath::IsNearlyEqual(Settings->GetResolutionScaleNormalized(), 1.0f));
	TestTrue(TEXT("Recommended settings enable every optional feature"), Settings->AreAllOptionalFeaturesEnabled());

	Settings->SetAllOptionalFeaturesEnabled(false);
	TestFalse(TEXT("The master switch disables every optional feature"), Settings->AreAnyOptionalFeaturesEnabled());
	for (uint8 Index = 0; Index < static_cast<uint8>(EUEGT1GraphicsFeature::Count); ++Index)
	{
		const EUEGT1GraphicsFeature Feature = static_cast<EUEGT1GraphicsFeature>(Index);
		TestFalse(*FString::Printf(TEXT("%s can be disabled"), *UUEGT1GameUserSettings::GetFeatureDisplayName(Feature).ToString()), Settings->IsFeatureEnabled(Feature));
	}

	Settings->ApplyNonResolutionSettings();
	const IConsoleVariable* Fog = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Fog"));
	const IConsoleVariable* Bloom = IConsoleManager::Get().FindConsoleVariable(TEXT("r.BloomQuality"));
	const IConsoleVariable* Lumen = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.DiffuseIndirect.Allow"));
	TestTrue(TEXT("The fog renderer is disabled by the feature switch"), Fog && Fog->GetInt() == 0);
	TestTrue(TEXT("Bloom is disabled by the feature switch"), Bloom && Bloom->GetInt() == 0);
	TestTrue(TEXT("Lumen GI is disabled by the feature switch"), Lumen && Lumen->GetInt() == 0);

	Settings->SetRecommendedDefaults();
	Settings->ApplyNonResolutionSettings();
	TestTrue(TEXT("The master switch re-enables every optional feature"), Settings->AreAllOptionalFeaturesEnabled());
	TestTrue(TEXT("The fog renderer is restored by the master switch"), Fog && Fog->GetInt() > 0);
	TestTrue(TEXT("Bloom is restored by the master switch"), Bloom && Bloom->GetInt() > 0);
	TestTrue(TEXT("Lumen GI is restored by the master switch"), Lumen && Lumen->GetInt() > 0);
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUEGT1GameplayFoundationTest,
	"UEGT1.Smoke.GameplayFoundation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUEGT1GameplayFoundationTest::RunTest(const FString& Parameters)
{
	const AUEGT1GameMode* GameMode = GetDefault<AUEGT1GameMode>();
	TestTrue(TEXT("The game mode uses the first-person explorer"), GameMode->DefaultPawnClass == AUEGT1ExplorerCharacter::StaticClass());
	TestTrue(TEXT("The game mode uses the project player controller"), GameMode->PlayerControllerClass == AUEGT1PlayerController::StaticClass());
	TestTrue(TEXT("The game mode uses milestone objective state"), GameMode->GameStateClass == AUEGT1MilestoneGameState::StaticClass());
	TestTrue(TEXT("The game mode uses the diegetic HUD"), GameMode->HUDClass == AUEGT1HUD::StaticClass());
	TestTrue(TEXT("Waystones implement the reusable interaction contract"), AUEGT1Waystone::StaticClass()->ImplementsInterface(UUEGT1Interactable::StaticClass()));
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUEGT1WorldLayoutTest,
	"UEGT1.Smoke.WorldLayout",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUEGT1WorldLayoutTest::RunTest(const FString& Parameters)
{
	const TArray<FName>& Ids = UEGT1WorldLayout::GetWaystoneIds();
	const TArray<FVector>& Locations = UEGT1WorldLayout::GetWaystoneLocations();
	TestEqual(TEXT("The milestone has three Waystones"), Ids.Num(), 3);
	TestEqual(TEXT("Every Waystone ID has a location"), Ids.Num(), Locations.Num());

	TSet<FName> UniqueIds(Ids);
	TestEqual(TEXT("Waystone IDs are unique"), UniqueIds.Num(), Ids.Num());
	const float WorldHalfExtent = (UEGT1WorldLayout::TileRadius + 0.5f) * UEGT1WorldLayout::TileSize;
	for (int32 Index = 0; Index < Locations.Num(); ++Index)
	{
		TestTrue(*FString::Printf(TEXT("Waystone %s is inside the authored world"), *Ids[Index].ToString()),
			FMath::Abs(Locations[Index].X) < WorldHalfExtent && FMath::Abs(Locations[Index].Y) < WorldHalfExtent);
		TestTrue(*FString::Printf(TEXT("Waystone %s is meaningfully separated from the sanctuary"), *Ids[Index].ToString()),
			FVector::Dist2D(Locations[Index], UEGT1WorldLayout::SanctuaryLocation) > 2500.0f);
		TestTrue(TEXT("The connecting trail remains reserved from biome clutter"),
			UEGT1WorldLayout::IsReservedGameplaySpace(Locations[Index] * 0.5f, 0.0f));
	}
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUEGT1WorldAssetTest,
	"UEGT1.Smoke.WorldAsset",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUEGT1WorldAssetTest::RunTest(const FString& Parameters)
{
#if WITH_EDITOR
	UWorld* MainWorld = LoadObject<UWorld>(nullptr, TEXT("/Game/Maps/Main.Main"));
	TestNotNull(TEXT("The Main world can be loaded"), MainWorld);
	if (MainWorld)
	{
		// A package-only load does not initialize UWorldPartition, but OFPA is persisted on the level.
		// The runtime smoke separately proves that all authored spatial content is available in game.
		TestTrue(TEXT("Main uses World Partition external actor packaging"), MainWorld->PersistentLevel && MainWorld->PersistentLevel->IsUsingExternalActors());
	}
#endif
	return !HasAnyErrors();
}

#endif
