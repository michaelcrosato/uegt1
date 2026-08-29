#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "UEGT1HUD.generated.h"

UCLASS()
class UEGT1_API AUEGT1HUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;
	void ToggleDiagnostics() { bShowDiagnostics = !bShowDiagnostics; }

private:
	void DrawObjectivePanel(float ScreenWidth, float ScreenHeight);
	void DrawPlayerStatusPanel(float ScreenWidth, float ScreenHeight);
	void DrawWorldMap(float ScreenWidth, float ScreenHeight);
	void EnsureWorldMapCache();
	void DrawInteractionPrompt(float ScreenWidth, float ScreenHeight);
	void DrawCrosshair(float ScreenWidth, float ScreenHeight);
	void DrawResidentThoughtBubbles(float ScreenWidth, float ScreenHeight);
	void DrawDeveloperModePanel(float ScreenWidth, float ScreenHeight);
	void DrawDiagnostics(float ScreenWidth, float ScreenHeight);
	void DrawSimulationInspector(float ScreenWidth, float ScreenHeight);

	bool bShowDiagnostics = false;
	int32 CachedWorldMapSeed = INDEX_NONE;
	TArray<FLinearColor> CachedWorldMapColors;
};
