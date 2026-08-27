#pragma once

#include "CoreMinimal.h"
#include "Settings/UEGT1GameUserSettings.h"
#include "Widgets/SCompoundWidget.h"

class AUEGT1PlayerController;
class SButton;
class SWidgetSwitcher;

class SUEGT1Menu final : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SUEGT1Menu) {}
		SLATE_ARGUMENT(TWeakObjectPtr<AUEGT1PlayerController>, OwnerController)
		SLATE_ARGUMENT(bool, IsInitialMenu)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual bool SupportsKeyboardFocus() const override { return true; }
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

	void ShowHomePage();
	void ShowGraphicsPage();
	void SetInitialFocus();

private:
	TSharedRef<SWidget> BuildHomePage();
	TSharedRef<SWidget> BuildGraphicsPage();
	TSharedRef<SWidget> MakeSectionHeader(const FText& Label, const FText& Detail = FText::GetEmpty());
	TSharedRef<SWidget> MakeCycleRow(const FText& Label, TFunction<FText()> ValueGetter, TFunction<void()> Previous, TFunction<void()> Next);
	TSharedRef<SWidget> MakeQualityRow(const FText& Label, TFunction<int32()> Getter, TFunction<void(int32)> Setter);
	TSharedRef<SWidget> MakeToggleRow(const FText& Label, TFunction<bool()> Getter, TFunction<void(bool)> Setter);
	TSharedRef<SButton> MakeActionButton(const FText& Label, const FSimpleDelegate& Action, bool bAccent = false);

	void ResumeGame();
	void ApplySettings();
	void DiscardSettings();
	void RestoreRecommendedSettings();
	void QuitGame();
	void LoadSignalGrove();
	void LoadTechDemo();
	void ToggleDeveloperMode();
	void ToggleDeveloperFlight();
	FText GetDeveloperModeLabel() const;
	FText GetDeveloperFlightLabel() const;
	void ToggleAllFeatures();
	FText GetAllFeaturesState() const;
	FText GetStatusText() const { return StatusText; }
	UUEGT1GameUserSettings* GetSettings() const;

	TWeakObjectPtr<AUEGT1PlayerController> OwnerController;
	TSharedPtr<SWidgetSwitcher> PageSwitcher;
	TSharedPtr<SButton> PrimaryButton;
	TSharedPtr<SButton> GraphicsFocusButton;
	FText StatusText;
	bool bIsInitialMenu = false;
	int32 CurrentPage = 0;
};
