#pragma once

#include "CoreMinimal.h"

enum class EUEGT1DayPhase : uint8
{
	Night,
	Dawn,
	Day,
	Dusk
};

struct FUEGT1DayNightState
{
	float Hour = 6.0f;
	float SunElevationDegrees = 0.0f;
	float SunAzimuthDegrees = 0.0f;
	float DaylightAlpha = 0.0f;
	float SunIntensityLux = 0.0f;
	float SkyLightIntensity = 0.02f;
	float ExposureEV100 = 12.8f;
	FLinearColor SunColor = FLinearColor::White;
	FLinearColor SkyColor = FLinearColor::White;
	FLinearColor FogColor = FLinearColor::Black;
	EUEGT1DayPhase Phase = EUEGT1DayPhase::Night;
};

namespace UEGT1DayNight
{
	UEGT1_API FUEGT1DayNightState Evaluate(float HourOfDay);
	UEGT1_API const TCHAR* LexToString(EUEGT1DayPhase Phase);
}
