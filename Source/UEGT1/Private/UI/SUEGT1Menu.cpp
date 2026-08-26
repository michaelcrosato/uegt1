#include "UI/SUEGT1Menu.h"

#include "Framework/Application/SlateApplication.h"
#include "InputCoreTypes.h"
#include "Player/UEGT1PlayerController.h"
#include "Brushes/SlateColorBrush.h"
#include "Styling/CoreStyle.h"
#include "UEGT1LogChannels.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	const FLinearColor Ink(0.008f, 0.025f, 0.024f, 0.98f);
	const FLinearColor Panel(0.015f, 0.075f, 0.07f, 0.96f);
	const FLinearColor PanelSoft(0.025f, 0.12f, 0.105f, 0.90f);
	const FLinearColor Signal(0.03f, 0.88f, 0.82f, 1.0f);
	const FLinearColor Amber(1.0f, 0.32f, 0.055f, 1.0f);
	const FLinearColor Paper(0.88f, 0.95f, 0.86f, 1.0f);
	const FLinearColor Muted(0.50f, 0.66f, 0.58f, 1.0f);

	const TArray<FIntPoint> SupportedResolutions = {
		FIntPoint(1280, 720), FIntPoint(1600, 900), FIntPoint(1920, 1080), FIntPoint(2560, 1440), FIntPoint(3840, 2160)
	};
	const TArray<float> SupportedFrameLimits = { 30.0f, 60.0f, 90.0f, 120.0f, 144.0f, 0.0f };
	const TArray<float> SupportedResolutionScales = { 50.0f, 67.0f, 75.0f, 85.0f, 100.0f };

	const FButtonStyle& GetActionButtonStyle(bool bAccent)
	{
		static const FButtonStyle AccentStyle = []()
		{
			FButtonStyle Style = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("Button");
			Style.SetNormal(FSlateColorBrush(Signal));
			Style.SetHovered(FSlateColorBrush(FLinearColor(0.08f, 1.0f, 0.92f, 1.0f)));
			Style.SetPressed(FSlateColorBrush(FLinearColor(0.02f, 0.68f, 0.64f, 1.0f)));
			return Style;
		}();
		static const FButtonStyle QuietStyle = []()
		{
			FButtonStyle Style = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("Button");
			Style.SetNormal(FSlateColorBrush(FLinearColor(0.055f, 0.18f, 0.16f, 1.0f)));
			Style.SetHovered(FSlateColorBrush(FLinearColor(0.07f, 0.28f, 0.24f, 1.0f)));
			Style.SetPressed(FSlateColorBrush(FLinearColor(0.03f, 0.12f, 0.11f, 1.0f)));
			return Style;
		}();
		return bAccent ? AccentStyle : QuietStyle;
	}

	const FButtonStyle& GetToggleButtonStyle()
	{
		static const FButtonStyle ToggleStyle = []()
		{
			FButtonStyle Style = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("Button");
			Style.SetNormal(FSlateColorBrush(FLinearColor::White));
			Style.SetHovered(FSlateColorBrush(FLinearColor(1.0f, 1.0f, 1.0f, 0.92f)));
			Style.SetPressed(FSlateColorBrush(FLinearColor(0.72f, 0.78f, 0.74f, 1.0f)));
			return Style;
		}();
		return ToggleStyle;
	}

	FText QualityText(int32 Quality)
	{
		static const TCHAR* Names[] = { TEXT("LOW"), TEXT("MEDIUM"), TEXT("HIGH"), TEXT("EPIC"), TEXT("CINEMATIC") };
		return Quality >= 0 && Quality < UE_ARRAY_COUNT(Names) ? FText::FromString(Names[Quality]) : NSLOCTEXT("UEGT1", "CustomQuality", "CUSTOM");
	}

	FText WindowModeText(EWindowMode::Type Mode)
	{
		switch (Mode)
		{
		case EWindowMode::Fullscreen: return NSLOCTEXT("UEGT1", "Fullscreen", "FULLSCREEN");
		case EWindowMode::Windowed: return NSLOCTEXT("UEGT1", "Windowed", "WINDOWED");
		default: return NSLOCTEXT("UEGT1", "Borderless", "BORDERLESS");
		}
	}

	int32 FindClosestIndex(const TArray<float>& Values, float Current)
	{
		int32 BestIndex = 0;
		float BestDistance = TNumericLimits<float>::Max();
		for (int32 Index = 0; Index < Values.Num(); ++Index)
		{
			const float Comparable = Values[Index] <= 0.0f ? 1000.0f : Values[Index];
			const float CurrentComparable = Current <= 0.0f ? 1000.0f : Current;
			const float Distance = FMath::Abs(Comparable - CurrentComparable);
			if (Distance < BestDistance)
			{
				BestDistance = Distance;
				BestIndex = Index;
			}
		}
		return BestIndex;
	}
}

void SUEGT1Menu::Construct(const FArguments& InArgs)
{
	OwnerController = InArgs._OwnerController;
	bIsInitialMenu = InArgs._IsInitialMenu;
	StatusText = NSLOCTEXT("UEGT1", "SettingsHint", "Changes are staged until APPLY & SAVE.");

	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.002f, 0.012f, 0.012f, 0.82f))
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(44.0f)
		[
			SNew(SBox)
			.WidthOverride(1380.0f)
			.HeightOverride(900.0f)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(Ink)
				.Padding(0.0f)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					.HAlign(HAlign_Left)
					[
						SNew(SBox)
						.WidthOverride(8.0f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(Signal)
						]
					]
					+ SOverlay::Slot()
					.Padding(56.0f, 42.0f, 48.0f, 38.0f)
					[
						SAssignNew(PageSwitcher, SWidgetSwitcher)
						.WidgetIndex(0)
						+ SWidgetSwitcher::Slot()[BuildHomePage()]
						+ SWidgetSwitcher::Slot()[BuildGraphicsPage()]
					]
				]
			]
		]
	];
}

TSharedRef<SWidget> SUEGT1Menu::BuildHomePage()
{
	TSharedRef<SButton> ContinueButton = MakeActionButton(
		bIsInitialMenu ? NSLOCTEXT("UEGT1", "EnterGrove", "ENTER THE GROVE") : NSLOCTEXT("UEGT1", "Resume", "RETURN TO THE GROVE"),
		FSimpleDelegate::CreateSP(this, &SUEGT1Menu::ResumeGame), true);
	PrimaryButton = ContinueButton;

	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(0.54f)
		.Padding(18.0f, 22.0f, 78.0f, 20.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("UEGT1", "SignalGroveTitle", "SIGNAL\nGROVE"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 66))
				.ColorAndOpacity(Paper)
				.LineHeightPercentage(0.78f)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(2.0f, 22.0f, 0.0f, 0.0f)
			[
				SNew(SBox).WidthOverride(92.0f).HeightOverride(5.0f)
				[
					SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(Amber)
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(2.0f, 28.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("UEGT1", "MenuLore", "THREE SIGNALS WAIT BEYOND THE SANCTUARY.\nFOLLOW THE AMBER PATHS. WAKE THE GROVE."))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 15))
				.ColorAndOpacity(Muted)
				.LineHeightPercentage(1.35f)
			]
			+ SVerticalBox::Slot().FillHeight(1.0f)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("UEGT1", "MenuVersion", "V0.2  •  SETTINGS MILESTONE"))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
				.ColorAndOpacity(FLinearColor(0.25f, 0.48f, 0.41f, 1.0f))
			]
		]
		+ SHorizontalBox::Slot()
		.FillWidth(0.46f)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(Panel)
			.Padding(44.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f, 0.0f, 8.0f)
				[
					SNew(STextBlock)
					.Text(bIsInitialMenu ? NSLOCTEXT("UEGT1", "MainMenu", "MAIN MENU") : NSLOCTEXT("UEGT1", "PausedMenu", "PAUSED"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
					.ColorAndOpacity(Signal)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 18.0f, 0.0f, 0.0f)[ContinueButton]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 12.0f, 0.0f, 0.0f)
				[
					MakeActionButton(NSLOCTEXT("UEGT1", "Graphics", "GRAPHICS & DISPLAY"), FSimpleDelegate::CreateSP(this, &SUEGT1Menu::ShowGraphicsPage))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 12.0f, 0.0f, 0.0f)
				[
					MakeActionButton(NSLOCTEXT("UEGT1", "QuitDesktop", "QUIT TO DESKTOP"), FSimpleDelegate::CreateSP(this, &SUEGT1Menu::QuitGame))
				]
				+ SVerticalBox::Slot().FillHeight(1.0f)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("UEGT1", "MenuNavigation", "MOUSE  •  ARROW KEYS  •  ENTER  •  ESC / START"))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
					.ColorAndOpacity(Muted)
				]
			]
		];
}

TSharedRef<SWidget> SUEGT1Menu::BuildGraphicsPage()
{
	UUEGT1GameUserSettings* Settings = GetSettings();
	check(Settings);

	auto ChangeWindowMode = [Settings](int32 Direction)
	{
		const TArray<EWindowMode::Type> Modes = { EWindowMode::WindowedFullscreen, EWindowMode::Fullscreen, EWindowMode::Windowed };
		int32 Index = Modes.IndexOfByKey(Settings->GetFullscreenMode());
		Index = (Index + Direction + Modes.Num()) % Modes.Num();
		Settings->SetFullscreenMode(Modes[Index]);
	};

	auto ChangeResolution = [Settings](int32 Direction)
	{
		int32 Index = SupportedResolutions.IndexOfByKey(Settings->GetScreenResolution());
		if (Index == INDEX_NONE)
		{
			Index = 2;
		}
		Index = FMath::Clamp(Index + Direction, 0, SupportedResolutions.Num() - 1);
		Settings->SetScreenResolution(SupportedResolutions[Index]);
	};

	auto ChangeFrameLimit = [Settings](int32 Direction)
	{
		int32 Index = FindClosestIndex(SupportedFrameLimits, Settings->GetFrameRateLimit());
		Index = FMath::Clamp(Index + Direction, 0, SupportedFrameLimits.Num() - 1);
		Settings->SetFrameRateLimit(SupportedFrameLimits[Index]);
	};

	auto ChangeResolutionScale = [Settings](int32 Direction)
	{
		float Normalized = 0.0f;
		float Current = 100.0f;
		float Minimum = 50.0f;
		float Maximum = 100.0f;
		Settings->GetResolutionScaleInformationEx(Normalized, Current, Minimum, Maximum);
		int32 Index = FindClosestIndex(SupportedResolutionScales, Current);
		Index = FMath::Clamp(Index + Direction, 0, SupportedResolutionScales.Num() - 1);
		Settings->SetResolutionScaleValueEx(SupportedResolutionScales[Index]);
	};

	TSharedRef<SVerticalBox> DisplayColumn = SNew(SVerticalBox);
	DisplayColumn->AddSlot().AutoHeight()[MakeSectionHeader(NSLOCTEXT("UEGT1", "DisplayHeader", "DISPLAY"), NSLOCTEXT("UEGT1", "DisplayDetail", "OUTPUT & PERFORMANCE"))];
	DisplayColumn->AddSlot().AutoHeight().Padding(0.0f, 10.0f, 0.0f, 0.0f)
	[
		MakeCycleRow(NSLOCTEXT("UEGT1", "WindowModeLabel", "WINDOW MODE"),
			[Settings]() { return WindowModeText(Settings->GetFullscreenMode()); },
			[ChangeWindowMode]() { ChangeWindowMode(-1); }, [ChangeWindowMode]() { ChangeWindowMode(1); })
	];
	DisplayColumn->AddSlot().AutoHeight()[MakeCycleRow(NSLOCTEXT("UEGT1", "ResolutionLabel", "RESOLUTION"),
		[Settings]() { const FIntPoint Size = Settings->GetScreenResolution(); return FText::FromString(FString::Printf(TEXT("%d × %d"), Size.X, Size.Y)); },
		[ChangeResolution]() { ChangeResolution(-1); }, [ChangeResolution]() { ChangeResolution(1); })];
	DisplayColumn->AddSlot().AutoHeight()[MakeCycleRow(NSLOCTEXT("UEGT1", "FrameLimitLabel", "FRAME LIMIT"),
		[Settings]() { const float Limit = Settings->GetFrameRateLimit(); return Limit <= 0.0f ? NSLOCTEXT("UEGT1", "Unlimited", "UNLIMITED") : FText::FromString(FString::Printf(TEXT("%.0f FPS"), Limit)); },
		[ChangeFrameLimit]() { ChangeFrameLimit(-1); }, [ChangeFrameLimit]() { ChangeFrameLimit(1); })];
	DisplayColumn->AddSlot().AutoHeight()[MakeCycleRow(NSLOCTEXT("UEGT1", "ResolutionScaleLabel", "RESOLUTION SCALE"),
		[Settings]() { float N, Current, Min, Max; Settings->GetResolutionScaleInformationEx(N, Current, Min, Max); return FText::FromString(FString::Printf(TEXT("%.0f%%"), Current)); },
		[ChangeResolutionScale]() { ChangeResolutionScale(-1); }, [ChangeResolutionScale]() { ChangeResolutionScale(1); })];
	DisplayColumn->AddSlot().AutoHeight()[MakeToggleRow(NSLOCTEXT("UEGT1", "VSyncLabel", "VERTICAL SYNC"),
		[Settings]() { return Settings->IsVSyncEnabled(); }, [Settings](bool bOn) { Settings->SetVSyncEnabled(bOn); })];
	DisplayColumn->AddSlot().AutoHeight()[MakeToggleRow(NSLOCTEXT("UEGT1", "DynamicResolutionLabel", "DYNAMIC RESOLUTION"),
		[Settings]() { return Settings->IsDynamicResolutionEnabled(); }, [Settings](bool bOn) { Settings->SetDynamicResolutionEnabled(bOn); })];

	DisplayColumn->AddSlot().AutoHeight().Padding(0.0f, 20.0f, 0.0f, 0.0f)
	[
		MakeSectionHeader(NSLOCTEXT("UEGT1", "QualityHeader", "QUALITY"), NSLOCTEXT("UEGT1", "QualityDetail", "LOW → CINEMATIC"))
	];
	DisplayColumn->AddSlot().AutoHeight().Padding(0.0f, 10.0f, 0.0f, 0.0f)[MakeQualityRow(NSLOCTEXT("UEGT1", "OverallQuality", "OVERALL PRESET"),
		[Settings]() { return Settings->GetOverallScalabilityLevel(); }, [Settings](int32 V) { Settings->SetOverallScalabilityLevel(FMath::Min(V, 3)); })];
	DisplayColumn->AddSlot().AutoHeight()[MakeQualityRow(NSLOCTEXT("UEGT1", "ViewDistanceQuality", "VIEW DISTANCE"),
		[Settings]() { return Settings->GetViewDistanceQuality(); }, [Settings](int32 V) { Settings->SetViewDistanceQuality(V); })];
	DisplayColumn->AddSlot().AutoHeight()[MakeQualityRow(NSLOCTEXT("UEGT1", "AntiAliasingQuality", "ANTI-ALIASING QUALITY"),
		[Settings]() { return Settings->GetAntiAliasingQuality(); }, [Settings](int32 V) { Settings->SetAntiAliasingQuality(V); })];
	DisplayColumn->AddSlot().AutoHeight()[MakeQualityRow(NSLOCTEXT("UEGT1", "ShadowQuality", "SHADOW QUALITY"),
		[Settings]() { return Settings->GetShadowQuality(); }, [Settings](int32 V) { Settings->SetShadowQuality(V); })];
	DisplayColumn->AddSlot().AutoHeight()[MakeQualityRow(NSLOCTEXT("UEGT1", "GIQuality", "GLOBAL ILLUMINATION QUALITY"),
		[Settings]() { return Settings->GetGlobalIlluminationQuality(); }, [Settings](int32 V) { Settings->SetGlobalIlluminationQuality(V); })];
	DisplayColumn->AddSlot().AutoHeight()[MakeQualityRow(NSLOCTEXT("UEGT1", "ReflectionQuality", "REFLECTION QUALITY"),
		[Settings]() { return Settings->GetReflectionQuality(); }, [Settings](int32 V) { Settings->SetReflectionQuality(V); })];
	DisplayColumn->AddSlot().AutoHeight()[MakeQualityRow(NSLOCTEXT("UEGT1", "TextureQuality", "TEXTURES"),
		[Settings]() { return Settings->GetTextureQuality(); }, [Settings](int32 V) { Settings->SetTextureQuality(V); })];
	DisplayColumn->AddSlot().AutoHeight()[MakeQualityRow(NSLOCTEXT("UEGT1", "EffectsQuality", "VISUAL EFFECTS"),
		[Settings]() { return Settings->GetVisualEffectQuality(); }, [Settings](int32 V) { Settings->SetVisualEffectQuality(V); })];
	DisplayColumn->AddSlot().AutoHeight()[MakeQualityRow(NSLOCTEXT("UEGT1", "PostQuality", "POST PROCESS"),
		[Settings]() { return Settings->GetPostProcessingQuality(); }, [Settings](int32 V) { Settings->SetPostProcessingQuality(V); })];
	DisplayColumn->AddSlot().AutoHeight()[MakeQualityRow(NSLOCTEXT("UEGT1", "FoliageQuality", "FOLIAGE DENSITY"),
		[Settings]() { return Settings->GetFoliageQuality(); }, [Settings](int32 V) { Settings->SetFoliageQuality(V); })];
	DisplayColumn->AddSlot().AutoHeight()[MakeQualityRow(NSLOCTEXT("UEGT1", "ShadingQuality", "SHADING"),
		[Settings]() { return Settings->GetShadingQuality(); }, [Settings](int32 V) { Settings->SetShadingQuality(V); })];

	TSharedRef<SVerticalBox> FeatureColumn = SNew(SVerticalBox);
	FeatureColumn->AddSlot().AutoHeight()[MakeSectionHeader(NSLOCTEXT("UEGT1", "FeaturesHeader", "FEATURE SWITCHES"), NSLOCTEXT("UEGT1", "FeaturesDetail", "EVERY OPTIONAL RENDERING GROUP"))];
	TSharedRef<SButton> AllFeaturesButton = MakeActionButton(GetAllFeaturesState(), FSimpleDelegate::CreateSP(this, &SUEGT1Menu::ToggleAllFeatures), true);
	AllFeaturesButton->SetContent(SNew(STextBlock)
		.Text_Lambda([this]() { return FText::Format(NSLOCTEXT("UEGT1", "AllFeaturesFormat", "ALL OPTIONAL EFFECTS  •  {0}"), GetAllFeaturesState()); })
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 13)).ColorAndOpacity(Ink).Justification(ETextJustify::Center));
	GraphicsFocusButton = AllFeaturesButton;
	FeatureColumn->AddSlot().AutoHeight().Padding(0.0f, 10.0f, 0.0f, 8.0f)[AllFeaturesButton];

	for (uint8 Index = 0; Index < static_cast<uint8>(EUEGT1GraphicsFeature::Count); ++Index)
	{
		const EUEGT1GraphicsFeature Feature = static_cast<EUEGT1GraphicsFeature>(Index);
		FeatureColumn->AddSlot().AutoHeight()
		[
			MakeToggleRow(UUEGT1GameUserSettings::GetFeatureDisplayName(Feature),
				[Settings, Feature]() { return Settings->IsFeatureEnabled(Feature); },
				[Settings, Feature](bool bOn) { Settings->SetFeatureEnabled(Feature, bOn); })
		];
	}

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.0f)
			[
				SNew(STextBlock).Text(NSLOCTEXT("UEGT1", "GraphicsSettingsTitle", "GRAPHICS & DISPLAY"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 30)).ColorAndOpacity(Paper)
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text_Lambda([this]() { return GetStatusText(); })
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11)).ColorAndOpacity(Muted)
			]
		]
		+ SVerticalBox::Slot().FillHeight(1.0f).Padding(0.0f, 24.0f, 0.0f, 18.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(0.5f).Padding(0.0f, 0.0f, 18.0f, 0.0f)
			[
				SNew(SScrollBox) + SScrollBox::Slot()[DisplayColumn]
			]
			+ SHorizontalBox::Slot().FillWidth(0.5f).Padding(18.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SScrollBox) + SScrollBox::Slot()[FeatureColumn]
			]
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.0f)[MakeActionButton(NSLOCTEXT("UEGT1", "ApplySave", "APPLY & SAVE"), FSimpleDelegate::CreateSP(this, &SUEGT1Menu::ApplySettings), true)]
			+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(12.0f, 0.0f)[MakeActionButton(NSLOCTEXT("UEGT1", "Recommended", "RESTORE RECOMMENDED"), FSimpleDelegate::CreateSP(this, &SUEGT1Menu::RestoreRecommendedSettings))]
			+ SHorizontalBox::Slot().FillWidth(1.0f)[MakeActionButton(NSLOCTEXT("UEGT1", "Discard", "BACK / DISCARD"), FSimpleDelegate::CreateSP(this, &SUEGT1Menu::DiscardSettings))]
		];
}

TSharedRef<SWidget> SUEGT1Menu::MakeSectionHeader(const FText& Label, const FText& Detail)
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.0f)
			[
				SNew(STextBlock).Text(Label).Font(FCoreStyle::GetDefaultFontStyle("Bold", 14)).ColorAndOpacity(Signal)
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(STextBlock).Text(Detail).Font(FCoreStyle::GetDefaultFontStyle("Regular", 10)).ColorAndOpacity(Muted)
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 8.0f, 0.0f, 0.0f)
		[
			SNew(SSeparator).Thickness(1.0f).ColorAndOpacity(FLinearColor(0.05f, 0.42f, 0.36f, 0.70f))
		];
}

TSharedRef<SWidget> SUEGT1Menu::MakeCycleRow(const FText& Label, TFunction<FText()> ValueGetter, TFunction<void()> Previous, TFunction<void()> Next)
{
	return SNew(SBox)
		.HeightOverride(41.0f)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(PanelSoft)
			.Padding(10.0f, 5.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
				[
					SNew(STextBlock).Text(Label).Font(FCoreStyle::GetDefaultFontStyle("Regular", 12)).ColorAndOpacity(Paper)
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(3.0f, 0.0f)
				[
					SNew(SButton).ContentPadding(FMargin(8.0f, 2.0f)).ButtonColorAndOpacity(Panel)
					.OnClicked_Lambda([Previous]() { Previous(); return FReply::Handled(); })
					[
						SNew(STextBlock).Text(FText::FromString(TEXT("‹"))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 16)).ColorAndOpacity(Signal)
					]
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(SBox).WidthOverride(150.0f)
					[
						SNew(STextBlock).Text_Lambda([ValueGetter]() { return ValueGetter(); })
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11)).ColorAndOpacity(Signal).Justification(ETextJustify::Center)
					]
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(3.0f, 0.0f)
				[
					SNew(SButton).ContentPadding(FMargin(8.0f, 2.0f)).ButtonColorAndOpacity(Panel)
					.OnClicked_Lambda([Next]() { Next(); return FReply::Handled(); })
					[
						SNew(STextBlock).Text(FText::FromString(TEXT("›"))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 16)).ColorAndOpacity(Signal)
					]
				]
			]
		];
}

TSharedRef<SWidget> SUEGT1Menu::MakeQualityRow(const FText& Label, TFunction<int32()> Getter, TFunction<void(int32)> Setter)
{
	return MakeCycleRow(Label,
		[Getter]() { return QualityText(Getter()); },
		[Getter, Setter]() { const int32 Current = Getter() < 0 ? 2 : Getter(); Setter(FMath::Clamp(Current - 1, 0, 4)); },
		[Getter, Setter]() { const int32 Current = Getter() < 0 ? 2 : Getter(); Setter(FMath::Clamp(Current + 1, 0, 4)); });
}

TSharedRef<SWidget> SUEGT1Menu::MakeToggleRow(const FText& Label, TFunction<bool()> Getter, TFunction<void(bool)> Setter)
{
	return SNew(SBox)
		.HeightOverride(41.0f)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(PanelSoft)
			.Padding(10.0f, 5.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
				[
					SNew(STextBlock).Text(Label).Font(FCoreStyle::GetDefaultFontStyle("Regular", 12)).ColorAndOpacity(Paper)
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SButton).ContentPadding(FMargin(18.0f, 3.0f))
					.ButtonStyle(&GetToggleButtonStyle())
					.ButtonColorAndOpacity_Lambda([Getter]() { return Getter() ? Signal : FLinearColor(0.09f, 0.14f, 0.12f, 1.0f); })
					.OnClicked_Lambda([Getter, Setter]() { Setter(!Getter()); return FReply::Handled(); })
					[
						SNew(STextBlock).Text_Lambda([Getter]() { return Getter() ? NSLOCTEXT("UEGT1", "On", "ON") : NSLOCTEXT("UEGT1", "Off", "OFF"); })
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
						.ColorAndOpacity_Lambda([Getter]() { return Getter() ? Ink : Muted; })
					]
				]
			]
		];
}

TSharedRef<SButton> SUEGT1Menu::MakeActionButton(const FText& Label, const FSimpleDelegate& Action, bool bAccent)
{
	return SNew(SButton)
		.ButtonStyle(&GetActionButtonStyle(bAccent))
		.ContentPadding(FMargin(18.0f, 14.0f))
		.HAlign(HAlign_Center)
		.OnClicked_Lambda([Action]() { Action.ExecuteIfBound(); return FReply::Handled(); })
		[
			SNew(STextBlock).Text(Label).Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
			.ColorAndOpacity(bAccent ? Ink : Paper).Justification(ETextJustify::Center)
		];
}

void SUEGT1Menu::ShowHomePage()
{
	CurrentPage = 0;
	PageSwitcher->SetActiveWidgetIndex(CurrentPage);
	StatusText = NSLOCTEXT("UEGT1", "SettingsHint", "Changes are staged until APPLY & SAVE.");
	FSlateApplication::Get().SetKeyboardFocus(PrimaryButton, EFocusCause::SetDirectly);
}

void SUEGT1Menu::ShowGraphicsPage()
{
	CurrentPage = 1;
	PageSwitcher->SetActiveWidgetIndex(CurrentPage);
	StatusText = NSLOCTEXT("UEGT1", "SettingsHint", "Changes are staged until APPLY & SAVE.");
	FSlateApplication::Get().SetKeyboardFocus(GraphicsFocusButton, EFocusCause::SetDirectly);
	UE_LOG(LogUEGT1, Display, TEXT("Graphics settings page opened."));
}

void SUEGT1Menu::SetInitialFocus()
{
	FSlateApplication::Get().SetKeyboardFocus(PrimaryButton, EFocusCause::SetDirectly);
}

FReply SUEGT1Menu::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	if (Key == EKeys::Escape || Key == EKeys::Gamepad_Special_Right)
	{
		if (CurrentPage == 1)
		{
			DiscardSettings();
		}
		else
		{
			ResumeGame();
		}
		return FReply::Handled();
	}
	return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}

void SUEGT1Menu::ResumeGame()
{
	if (AUEGT1PlayerController* Controller = OwnerController.Get())
	{
		Controller->CloseMenu();
	}
}

void SUEGT1Menu::ApplySettings()
{
	if (UUEGT1GameUserSettings* Settings = GetSettings())
	{
		Settings->ApplySettings(false);
		Settings->ConfirmVideoMode();
		Settings->SaveSettings();
		StatusText = NSLOCTEXT("UEGT1", "SettingsApplied", "Settings applied and saved.");
		UE_LOG(LogUEGT1, Display, TEXT("Graphics menu applied and saved user settings."));
	}
}

void SUEGT1Menu::DiscardSettings()
{
	if (UUEGT1GameUserSettings* Settings = GetSettings())
	{
		Settings->LoadSettings(true);
		Settings->ApplyNonResolutionSettings();
	}
	ShowHomePage();
}

void SUEGT1Menu::RestoreRecommendedSettings()
{
	if (UUEGT1GameUserSettings* Settings = GetSettings())
	{
		Settings->SetRecommendedDefaults();
		StatusText = NSLOCTEXT("UEGT1", "RecommendedStaged", "Recommended 1080p settings staged.");
	}
}

void SUEGT1Menu::QuitGame()
{
	if (AUEGT1PlayerController* Controller = OwnerController.Get())
	{
		Controller->RequestQuitFromMenu();
	}
}

void SUEGT1Menu::ToggleAllFeatures()
{
	if (UUEGT1GameUserSettings* Settings = GetSettings())
	{
		Settings->SetAllOptionalFeaturesEnabled(!Settings->AreAllOptionalFeaturesEnabled());
	}
}

FText SUEGT1Menu::GetAllFeaturesState() const
{
	const UUEGT1GameUserSettings* Settings = GetSettings();
	if (!Settings || Settings->AreAllOptionalFeaturesEnabled())
	{
		return NSLOCTEXT("UEGT1", "AllOn", "ALL ON");
	}
	return Settings->AreAnyOptionalFeaturesEnabled() ? NSLOCTEXT("UEGT1", "Mixed", "CUSTOM") : NSLOCTEXT("UEGT1", "AllOff", "ALL OFF");
}

UUEGT1GameUserSettings* SUEGT1Menu::GetSettings() const
{
	return UUEGT1GameUserSettings::Get();
}
