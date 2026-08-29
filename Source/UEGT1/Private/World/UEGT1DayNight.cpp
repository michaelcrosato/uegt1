#include "World/UEGT1DayNight.h"

namespace
{
	float SmoothStep(float Minimum, float Maximum, float Value)
	{
		const float Alpha = FMath::Clamp((Value - Minimum) / FMath::Max(Maximum - Minimum, KINDA_SMALL_NUMBER), 0.0f, 1.0f);
		return Alpha * Alpha * (3.0f - 2.0f * Alpha);
	}
}

FUEGT1DayNightState UEGT1DayNight::Evaluate(float HourOfDay)
{
	FUEGT1DayNightState State;
	State.Hour = FMath::Fmod(FMath::Fmod(HourOfDay, 24.0f) + 24.0f, 24.0f);
	const float SolarPhase = (State.Hour - 6.0f) / 24.0f * 2.0f * PI;
	const float SolarHeight = FMath::Sin(SolarPhase);
	State.SunElevationDegrees = SolarHeight * 68.0f;
	State.SunAzimuthDegrees = FMath::Fmod(82.0f + (State.Hour - 6.0f) / 24.0f * 360.0f + 360.0f, 360.0f);
	State.DaylightAlpha = SmoothStep(-0.08f, 0.40f, SolarHeight);

	const float HorizonWarmth = 1.0f - SmoothStep(0.05f, 0.52f, FMath::Abs(SolarHeight));
	const FLinearColor HorizonSun(1.0f, 0.30f, 0.10f, 1.0f);
	const FLinearColor DaySun(1.0f, 0.83f, 0.63f, 1.0f);
	State.SunColor = FLinearColor::LerpUsingHSV(DaySun, HorizonSun, HorizonWarmth);
	State.SunIntensityLux = 65000.0f * FMath::Pow(State.DaylightAlpha, 1.35f);

	const FLinearColor NightSky(0.32f, 0.46f, 0.80f, 1.0f);
	const FLinearColor DawnSky(0.92f, 0.42f, 0.22f, 1.0f);
	const FLinearColor DaySky(0.80f, 0.90f, 1.0f, 1.0f);
	const FLinearColor TwilightSky = FLinearColor::LerpUsingHSV(NightSky, DawnSky, SmoothStep(-0.20f, 0.04f, SolarHeight));
	State.SkyColor = FLinearColor::LerpUsingHSV(TwilightSky, DaySky, SmoothStep(0.02f, 0.45f, SolarHeight));
	State.SkyLightIntensity = FMath::Lerp(0.12f, 0.88f, FMath::Pow(State.DaylightAlpha, 0.85f));
	const float NightToDawn = SmoothStep(0.0f, 0.08f, State.DaylightAlpha);
	const float DawnToDay = SmoothStep(0.08f, 0.50f, State.DaylightAlpha);
	State.ExposureEV100 = FMath::Lerp(FMath::Lerp(5.5f, 9.5f, NightToDawn), 12.8f, DawnToDay);

	const FLinearColor NightFog(0.008f, 0.016f, 0.045f, 1.0f);
	const FLinearColor DawnFog(0.48f, 0.20f, 0.10f, 1.0f);
	const FLinearColor DayFog(0.62f, 0.75f, 0.82f, 1.0f);
	State.FogColor = FLinearColor::LerpUsingHSV(
		FLinearColor::LerpUsingHSV(NightFog, DawnFog, SmoothStep(-0.20f, 0.04f, SolarHeight)),
		DayFog, SmoothStep(0.04f, 0.42f, SolarHeight));

	if (State.Hour >= 5.0f && State.Hour < 8.0f)
	{
		State.Phase = EUEGT1DayPhase::Dawn;
	}
	else if (State.Hour >= 8.0f && State.Hour < 17.0f)
	{
		State.Phase = EUEGT1DayPhase::Day;
	}
	else if (State.Hour >= 17.0f && State.Hour < 21.0f)
	{
		State.Phase = EUEGT1DayPhase::Dusk;
	}
	else
	{
		State.Phase = EUEGT1DayPhase::Night;
	}
	return State;
}

const TCHAR* UEGT1DayNight::LexToString(EUEGT1DayPhase Phase)
{
	switch (Phase)
	{
	case EUEGT1DayPhase::Dawn: return TEXT("Dawn");
	case EUEGT1DayPhase::Day: return TEXT("Day");
	case EUEGT1DayPhase::Dusk: return TEXT("Dusk");
	default: return TEXT("Night");
	}
}
