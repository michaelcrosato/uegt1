#include "Settings/UEGT1GameUserSettings.h"

#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "UEGT1LogChannels.h"

FUEGT1GraphicsSettingsApplied UUEGT1GameUserSettings::OnGraphicsSettingsApplied;

namespace
{
	void SetIntCVar(const TCHAR* Name, int32 Value)
	{
		if (IConsoleVariable* Variable = IConsoleManager::Get().FindConsoleVariable(Name))
		{
			// Match UE's scalability priority so the base settings pass can restore an
			// enabled feature before this layer reapplies any explicit OFF switch.
			Variable->Set(Value, ECVF_SetByScalability);
		}
	}

	template <uint32 Count>
	int32 QualityValue(const int32 (&Values)[Count], int32 Quality)
	{
		return Values[FMath::Clamp(Quality, 0, static_cast<int32>(Count) - 1)];
	}
}

UUEGT1GameUserSettings* UUEGT1GameUserSettings::Get()
{
	return GEngine ? Cast<UUEGT1GameUserSettings>(GEngine->GetGameUserSettings()) : nullptr;
}

void UUEGT1GameUserSettings::ApplyNonResolutionSettings()
{
	Super::ApplyNonResolutionSettings();
	ApplyFeatureCVars();
	OnGraphicsSettingsApplied.Broadcast();
	UE_LOG(LogUEGT1, Display, TEXT("Graphics settings applied: Quality=%d Resolution=%s Scale=%.0f VSync=%s Optional=%s"),
		GetOverallScalabilityLevel(), *GetScreenResolution().ToString(), GetResolutionScaleNormalized() * 100.0f,
		IsVSyncEnabled() ? TEXT("On") : TEXT("Off"), AreAllOptionalFeaturesEnabled() ? TEXT("AllOn") : AreAnyOptionalFeaturesEnabled() ? TEXT("Custom") : TEXT("AllOff"));
}

void UUEGT1GameUserSettings::SetToDefaults()
{
	Super::SetToDefaults();
	SetRecommendedDefaults();
}

void UUEGT1GameUserSettings::SetRecommendedDefaults()
{
	SetScreenResolution(FIntPoint(1920, 1080));
	SetFullscreenMode(EWindowMode::WindowedFullscreen);
	SetFrameRateLimit(60.0f);
	SetVSyncEnabled(true);
	SetDynamicResolutionEnabled(false);
	SetOverallScalabilityLevel(2);
	SetTextureQuality(3);
	SetResolutionScaleNormalized(1.0f);
	SetAllOptionalFeaturesEnabled(true);
}

bool UUEGT1GameUserSettings::IsFeatureEnabled(EUEGT1GraphicsFeature Feature) const
{
	switch (Feature)
	{
	case EUEGT1GraphicsFeature::AntiAliasing: return bAntiAliasingEnabled;
	case EUEGT1GraphicsFeature::Shadows: return bShadowsEnabled;
	case EUEGT1GraphicsFeature::GlobalIllumination: return bGlobalIlluminationEnabled;
	case EUEGT1GraphicsFeature::Reflections: return bReflectionsEnabled;
	case EUEGT1GraphicsFeature::AmbientOcclusion: return bAmbientOcclusionEnabled;
	case EUEGT1GraphicsFeature::Bloom: return bBloomEnabled;
	case EUEGT1GraphicsFeature::MotionBlur: return bMotionBlurEnabled;
	case EUEGT1GraphicsFeature::DepthOfField: return bDepthOfFieldEnabled;
	case EUEGT1GraphicsFeature::LensFlares: return bLensFlaresEnabled;
	case EUEGT1GraphicsFeature::Fog: return bFogEnabled;
	case EUEGT1GraphicsFeature::Foliage: return bFoliageEnabled;
	default: return false;
	}
}

void UUEGT1GameUserSettings::SetFeatureEnabled(EUEGT1GraphicsFeature Feature, bool bEnabled)
{
	switch (Feature)
	{
	case EUEGT1GraphicsFeature::AntiAliasing: bAntiAliasingEnabled = bEnabled; break;
	case EUEGT1GraphicsFeature::Shadows: bShadowsEnabled = bEnabled; break;
	case EUEGT1GraphicsFeature::GlobalIllumination: bGlobalIlluminationEnabled = bEnabled; break;
	case EUEGT1GraphicsFeature::Reflections: bReflectionsEnabled = bEnabled; break;
	case EUEGT1GraphicsFeature::AmbientOcclusion: bAmbientOcclusionEnabled = bEnabled; break;
	case EUEGT1GraphicsFeature::Bloom: bBloomEnabled = bEnabled; break;
	case EUEGT1GraphicsFeature::MotionBlur: bMotionBlurEnabled = bEnabled; break;
	case EUEGT1GraphicsFeature::DepthOfField: bDepthOfFieldEnabled = bEnabled; break;
	case EUEGT1GraphicsFeature::LensFlares: bLensFlaresEnabled = bEnabled; break;
	case EUEGT1GraphicsFeature::Fog: bFogEnabled = bEnabled; break;
	case EUEGT1GraphicsFeature::Foliage: bFoliageEnabled = bEnabled; break;
	default: break;
	}
}

void UUEGT1GameUserSettings::SetAllOptionalFeaturesEnabled(bool bEnabled)
{
	for (uint8 Index = 0; Index < static_cast<uint8>(EUEGT1GraphicsFeature::Count); ++Index)
	{
		SetFeatureEnabled(static_cast<EUEGT1GraphicsFeature>(Index), bEnabled);
	}
}

bool UUEGT1GameUserSettings::AreAllOptionalFeaturesEnabled() const
{
	for (uint8 Index = 0; Index < static_cast<uint8>(EUEGT1GraphicsFeature::Count); ++Index)
	{
		if (!IsFeatureEnabled(static_cast<EUEGT1GraphicsFeature>(Index)))
		{
			return false;
		}
	}
	return true;
}

bool UUEGT1GameUserSettings::AreAnyOptionalFeaturesEnabled() const
{
	for (uint8 Index = 0; Index < static_cast<uint8>(EUEGT1GraphicsFeature::Count); ++Index)
	{
		if (IsFeatureEnabled(static_cast<EUEGT1GraphicsFeature>(Index)))
		{
			return true;
		}
	}
	return false;
}

FText UUEGT1GameUserSettings::GetFeatureDisplayName(EUEGT1GraphicsFeature Feature)
{
	switch (Feature)
	{
	case EUEGT1GraphicsFeature::AntiAliasing: return NSLOCTEXT("UEGT1", "AntiAliasing", "ANTI-ALIASING");
	case EUEGT1GraphicsFeature::Shadows: return NSLOCTEXT("UEGT1", "Shadows", "SHADOWS");
	case EUEGT1GraphicsFeature::GlobalIllumination: return NSLOCTEXT("UEGT1", "GlobalIllumination", "LUMEN GLOBAL ILLUMINATION");
	case EUEGT1GraphicsFeature::Reflections: return NSLOCTEXT("UEGT1", "Reflections", "LUMEN REFLECTIONS");
	case EUEGT1GraphicsFeature::AmbientOcclusion: return NSLOCTEXT("UEGT1", "AmbientOcclusion", "AMBIENT OCCLUSION");
	case EUEGT1GraphicsFeature::Bloom: return NSLOCTEXT("UEGT1", "Bloom", "BLOOM");
	case EUEGT1GraphicsFeature::MotionBlur: return NSLOCTEXT("UEGT1", "MotionBlur", "MOTION BLUR");
	case EUEGT1GraphicsFeature::DepthOfField: return NSLOCTEXT("UEGT1", "DepthOfField", "DEPTH OF FIELD");
	case EUEGT1GraphicsFeature::LensFlares: return NSLOCTEXT("UEGT1", "LensFlares", "LENS FLARES");
	case EUEGT1GraphicsFeature::Fog: return NSLOCTEXT("UEGT1", "Fog", "FOG");
	case EUEGT1GraphicsFeature::Foliage: return NSLOCTEXT("UEGT1", "Foliage", "FOLIAGE");
	default: return FText::GetEmpty();
	}
}

void UUEGT1GameUserSettings::ApplyFeatureCVars() const
{
	static constexpr int32 ShadowValues[] = { 0, 3, 5, 5, 5 };
	static constexpr int32 LumenGIValues[] = { 0, 1, 1, 1, 1 };
	static constexpr int32 LumenReflectionValues[] = { 0, 0, 1, 1, 1 };
	static constexpr int32 AmbientOcclusionValues[] = { 0, -1, -1, -1, -1 };
	static constexpr int32 BloomValues[] = { 4, 4, 5, 5, 5 };
	static constexpr int32 MotionBlurValues[] = { 0, 3, 3, 4, 4 };
	static constexpr int32 DepthOfFieldValues[] = { 0, 1, 2, 2, 4 };
	static constexpr int32 LensFlareValues[] = { 0, 0, 2, 2, 3 };
	static constexpr int32 VolumetricFogValues[] = { 0, 0, 1, 1, 1 };

	const int32 ShadowQuality = GetShadowQuality();
	const int32 GIQuality = GetGlobalIlluminationQuality();
	const int32 ReflectionQuality = GetReflectionQuality();
	const int32 PostQuality = GetPostProcessingQuality();

	SetIntCVar(TEXT("r.AntiAliasingMethod"), bAntiAliasingEnabled ? 4 : 0);
	SetIntCVar(TEXT("r.ShadowQuality"), bShadowsEnabled ? QualityValue(ShadowValues, ShadowQuality) : 0);
	SetIntCVar(TEXT("r.Lumen.DiffuseIndirect.Allow"), bGlobalIlluminationEnabled ? QualityValue(LumenGIValues, GIQuality) : 0);
	SetIntCVar(TEXT("r.Lumen.Reflections.Allow"), bReflectionsEnabled ? QualityValue(LumenReflectionValues, ReflectionQuality) : 0);
	SetIntCVar(TEXT("r.AmbientOcclusionLevels"), bAmbientOcclusionEnabled ? QualityValue(AmbientOcclusionValues, PostQuality) : 0);
	SetIntCVar(TEXT("r.BloomQuality"), bBloomEnabled ? QualityValue(BloomValues, PostQuality) : 0);
	SetIntCVar(TEXT("r.MotionBlurQuality"), bMotionBlurEnabled ? QualityValue(MotionBlurValues, PostQuality) : 0);
	SetIntCVar(TEXT("r.DepthOfFieldQuality"), bDepthOfFieldEnabled ? QualityValue(DepthOfFieldValues, PostQuality) : 0);
	SetIntCVar(TEXT("r.LensFlareQuality"), bLensFlaresEnabled ? QualityValue(LensFlareValues, PostQuality) : 0);
	SetIntCVar(TEXT("r.Fog"), bFogEnabled ? 1 : 0);
	SetIntCVar(TEXT("r.VolumetricFog"), bFogEnabled ? QualityValue(VolumetricFogValues, ShadowQuality) : 0);
}
