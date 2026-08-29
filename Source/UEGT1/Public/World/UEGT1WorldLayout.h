#pragma once

#include "CoreMinimal.h"
#include "World/UEGT1RegionTypes.h"

namespace UEGT1WorldLayout
{
	UEGT1_API int32 GetWorldSeed();
	UEGT1_API int32 GetTileRadius();
	UEGT1_API int32 GetMinTileX();
	UEGT1_API int32 GetMaxTileX();
	UEGT1_API int32 GetMinTileY();
	UEGT1_API int32 GetMaxTileY();
	UEGT1_API int32 GetExpectedTileCount();
	UEGT1_API float GetTileSize();
	UEGT1_API float GetWorldHalfExtent();
	UEGT1_API float GetWorldMinX();
	UEGT1_API float GetWorldMaxX();
	UEGT1_API float GetWorldMinY();
	UEGT1_API float GetWorldMaxY();
	UEGT1_API float GetSeaLevel();

	UEGT1_API const FVector& GetSanctuaryLocation();
	UEGT1_API const FVector& GetPlayerStartLocation();
	UEGT1_API const TArray<FVector>& GetWaystoneLocations();
	UEGT1_API const TArray<FName>& GetWaystoneIds();

	UEGT1_API FUEGT1RegionSample SampleRegion(const FVector& WorldPosition);
	UEGT1_API float GetTemperatureCelsius(const FVector& WorldPosition, float HourOfDay);
	UEGT1_API bool IsReservedGameplaySpace(const FVector& WorldPosition, float Radius);
	UEGT1_API bool IsPrimaryRoute(const FVector& WorldPosition, float Radius);
}
