#include "UI/UEGT1HUD.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Gameplay/UEGT1MilestoneGameState.h"
#include "Interaction/UEGT1InteractionComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/UEGT1ExplorerCharacter.h"
#include "Player/UEGT1PlayerController.h"
#include "World/UEGT1Palette.h"
#include "World/UEGT1WorldLayout.h"

void AUEGT1HUD::DrawHUD()
{
	Super::DrawHUD();
	if (!Canvas)
	{
		return;
	}
	if (const AUEGT1PlayerController* PlayerController = Cast<AUEGT1PlayerController>(GetOwningPlayerController());
		PlayerController && PlayerController->IsMenuOpen())
	{
		return;
	}

	const float ScreenWidth = Canvas->ClipX;
	const float ScreenHeight = Canvas->ClipY;
	DrawObjectivePanel(ScreenWidth, ScreenHeight);
	DrawDeveloperModePanel(ScreenWidth, ScreenHeight);
	DrawInteractionPrompt(ScreenWidth, ScreenHeight);
	DrawCrosshair(ScreenWidth, ScreenHeight);
	if (bShowDiagnostics)
	{
		DrawDiagnostics(ScreenWidth, ScreenHeight);
	}
}

void AUEGT1HUD::DrawObjectivePanel(float ScreenWidth, float ScreenHeight)
{
	const bool bTechDemo = UGameplayStatics::GetCurrentLevelName(GetWorld(), true).Contains(TEXT("TechDemo"));
	const AUEGT1MilestoneGameState* State = GetWorld()->GetGameState<AUEGT1MilestoneGameState>();
	const int32 Activated = State ? State->GetActivatedCount() : 0;
	const int32 Total = State ? State->GetTotalCount() : 3;
	const bool bComplete = State && State->IsMilestoneComplete();

	DrawRect(UEGT1Palette::Ink, 30.0f, 28.0f, bTechDemo ? 430.0f : 370.0f, 105.0f);
	DrawRect(bComplete ? UEGT1Palette::Signal : UEGT1Palette::Amber, 30.0f, 28.0f, 6.0f, 105.0f);
	DrawText(bTechDemo ? TEXT("LUMEN WILDS") : TEXT("SIGNAL GROVE"), UEGT1Palette::Paper, 52.0f, 43.0f, GEngine->GetMediumFont(), 1.0f, false);
	DrawText(bTechDemo ? TEXT("UE5 real-time environment showcase") : (bComplete ? TEXT("Sanctuary restored") : TEXT("Restore the three Waystones")),
		bComplete ? UEGT1Palette::Signal : UEGT1Palette::Paper, 52.0f, 75.0f, GEngine->GetSmallFont(), 1.05f, false);
	DrawText(bTechDemo ? TEXT("EXPLORE  •  FLY  •  INSPECT") : FString::Printf(TEXT("SIGNALS  %d / %d"), Activated, FMath::Max(Total, 3)),
		bComplete ? UEGT1Palette::Signal : UEGT1Palette::Amber, 52.0f, 101.0f, GEngine->GetSmallFont(), 0.9f, false);

	if (GetWorld()->GetTimeSeconds() < 12.0f)
	{
		DrawText(TEXT("WASD move   SHIFT sprint   SPACE jump/up   CTRL down   F8 dev mode   F9 flight   ESC menu   F3 diagnostics"),
			FLinearColor(0.72f, 0.82f, 0.73f, 0.95f), 32.0f, ScreenHeight - 42.0f, GEngine->GetSmallFont(), 0.85f, false);
	}

	if (bComplete && !bTechDemo)
	{
		const FString CompletionText = TEXT("THE GROVE ANSWERS");
		float TextWidth = 0.0f;
		float TextHeight = 0.0f;
		GetTextSize(CompletionText, TextWidth, TextHeight, GEngine->GetLargeFont(), 1.0f);
		DrawRect(FLinearColor(0.01f, 0.08f, 0.075f, 0.86f), ScreenWidth * 0.5f - 260.0f, 70.0f, 520.0f, 62.0f);
		DrawText(CompletionText, UEGT1Palette::Signal, ScreenWidth * 0.5f - TextWidth * 0.5f, 83.0f, GEngine->GetLargeFont(), 1.0f, false);
	}
}

void AUEGT1HUD::DrawDeveloperModePanel(float ScreenWidth, float ScreenHeight)
{
	const AUEGT1ExplorerCharacter* Character = Cast<AUEGT1ExplorerCharacter>(GetOwningPawn());
	if (!Character || !Character->IsDeveloperModeEnabled())
	{
		return;
	}

	const FString State = Character->IsDeveloperFlying()
		? TEXT("DEV MODE  •  INVINCIBLE  •  4.2K SPEED  •  FLIGHT")
		: TEXT("DEV MODE  •  INVINCIBLE  •  4.2K SPEED  •  F9 TO FLY");
	float Width = 0.0f;
	float Height = 0.0f;
	GetTextSize(State, Width, Height, GEngine->GetSmallFont(), 0.9f);
	DrawRect(FLinearColor(0.12f, 0.015f, 0.04f, 0.90f), ScreenWidth * 0.5f - Width * 0.5f - 18.0f, 28.0f, Width + 36.0f, 34.0f);
	DrawRect(FLinearColor(1.0f, 0.15f, 0.35f, 1.0f), ScreenWidth * 0.5f - Width * 0.5f - 18.0f, 28.0f, 5.0f, 34.0f);
	DrawText(State, FLinearColor(1.0f, 0.72f, 0.77f, 1.0f), ScreenWidth * 0.5f - Width * 0.5f, 37.0f, GEngine->GetSmallFont(), 0.9f, false);
}

void AUEGT1HUD::DrawInteractionPrompt(float ScreenWidth, float ScreenHeight)
{
	const AUEGT1ExplorerCharacter* Character = Cast<AUEGT1ExplorerCharacter>(GetOwningPawn());
	const UUEGT1InteractionComponent* Interaction = Character ? Character->GetInteractionComponent() : nullptr;
	if (!Interaction || !Interaction->HasValidFocus())
	{
		return;
	}

	const FString Prompt = Interaction->GetFocusedPrompt().ToString();
	float Width = 0.0f;
	float Height = 0.0f;
	GetTextSize(Prompt, Width, Height, GEngine->GetMediumFont(), 1.0f);
	DrawRect(FLinearColor(0.01f, 0.04f, 0.035f, 0.88f), ScreenWidth * 0.5f - Width * 0.5f - 18.0f, ScreenHeight * 0.70f - 10.0f, Width + 36.0f, Height + 20.0f);
	DrawText(Prompt, UEGT1Palette::Paper, ScreenWidth * 0.5f - Width * 0.5f, ScreenHeight * 0.70f, GEngine->GetMediumFont(), 1.0f, false);
}

void AUEGT1HUD::DrawCrosshair(float ScreenWidth, float ScreenHeight)
{
	const float CenterX = ScreenWidth * 0.5f;
	const float CenterY = ScreenHeight * 0.5f;
	const FLinearColor Color(0.82f, 0.93f, 0.84f, 0.85f);
	DrawRect(Color, CenterX - 1.0f, CenterY - 8.0f, 2.0f, 5.0f);
	DrawRect(Color, CenterX - 1.0f, CenterY + 3.0f, 2.0f, 5.0f);
	DrawRect(Color, CenterX - 8.0f, CenterY - 1.0f, 5.0f, 2.0f);
	DrawRect(Color, CenterX + 3.0f, CenterY - 1.0f, 5.0f, 2.0f);
}

void AUEGT1HUD::DrawDiagnostics(float ScreenWidth, float ScreenHeight)
{
	const APawn* Pawn = GetOwningPawn();
	const AUEGT1ExplorerCharacter* Character = Cast<AUEGT1ExplorerCharacter>(Pawn);
	const UUEGT1InteractionComponent* Interaction = Character ? Character->GetInteractionComponent() : nullptr;
	const float Fps = GetWorld()->GetDeltaSeconds() > SMALL_NUMBER ? 1.0f / GetWorld()->GetDeltaSeconds() : 0.0f;
	const FVector Position = Pawn ? Pawn->GetActorLocation() : FVector::ZeroVector;
	const FUEGT1RegionSample Region = UEGT1WorldLayout::SampleRegion(Position);
	const FString Focus = Interaction && Interaction->GetFocusedActor() ? Interaction->GetFocusedActor()->GetName() : TEXT("None");

	DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.80f), ScreenWidth - 415.0f, 28.0f, 385.0f, 178.0f);
	DrawText(TEXT("UEGT1 REGION DIAGNOSTICS"), UEGT1Palette::Signal, ScreenWidth - 397.0f, 42.0f, GEngine->GetSmallFont(), 0.9f, false);
	DrawText(FString::Printf(TEXT("FPS %.0f   Seed %d"), Fps, UEGT1WorldLayout::GetWorldSeed()), UEGT1Palette::Paper, ScreenWidth - 397.0f, 68.0f, GEngine->GetSmallFont(), 0.85f, false);
	DrawText(FString::Printf(TEXT("XYZ %.0f  %.0f  %.0f"), Position.X, Position.Y, Position.Z), UEGT1Palette::Paper, ScreenWidth - 397.0f, 91.0f, GEngine->GetSmallFont(), 0.85f, false);
	DrawText(FString::Printf(TEXT("Region %s   Ground %.0f   Water %.0f"), LexToString(Region.GetDominantBiome()), Region.SurfaceHeight, Region.WaterDepth),
		UEGT1Palette::Paper, ScreenWidth - 397.0f, 114.0f, GEngine->GetSmallFont(), 0.82f, false);
	DrawText(FString::Printf(TEXT("Temp %.2f   Moisture %.2f"), Region.Temperature, Region.Moisture),
		UEGT1Palette::Paper, ScreenWidth - 397.0f, 137.0f, GEngine->GetSmallFont(), 0.82f, false);
	DrawText(FString::Printf(TEXT("Focus %s"), *Focus), UEGT1Palette::Paper, ScreenWidth - 397.0f, 160.0f, GEngine->GetSmallFont(), 0.82f, false);
	DrawText(TEXT("Trace: uegt1.Debug.DrawInteraction 1"), FLinearColor(0.65f, 0.72f, 0.67f, 1.0f), ScreenWidth - 397.0f, 183.0f, GEngine->GetSmallFont(), 0.72f, false);
}
