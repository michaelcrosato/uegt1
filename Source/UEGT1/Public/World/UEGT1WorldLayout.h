#pragma once

#include "CoreMinimal.h"

namespace UEGT1WorldLayout
{
	inline constexpr int32 WorldSeed = 7319;
	inline constexpr int32 TileRadius = 2;
	inline constexpr float TileSize = 3200.0f;
	inline const FVector SanctuaryLocation(0.0f, 0.0f, 0.0f);
	inline const FVector PlayerStartLocation(0.0f, -1300.0f, 100.0f);

	inline const TArray<FVector>& GetWaystoneLocations()
	{
		static const TArray<FVector> Locations = {
			FVector(5200.0f, 3900.0f, 0.0f),
			FVector(-6100.0f, 2800.0f, 0.0f),
			FVector(900.0f, -7000.0f, 0.0f)
		};
		return Locations;
	}

	inline const TArray<FName>& GetWaystoneIds()
	{
		static const TArray<FName> Ids = { TEXT("EastRise"), TEXT("WestHollow"), TEXT("SouthWatch") };
		return Ids;
	}

	inline bool IsReservedGameplaySpace(const FVector& WorldPosition, float Radius)
	{
		if (FVector::Dist2D(WorldPosition, SanctuaryLocation) < 2200.0f + Radius)
		{
			return true;
		}

		for (const FVector& WaystoneLocation : GetWaystoneLocations())
		{
			if (FVector::Dist2D(WorldPosition, WaystoneLocation) < 650.0f + Radius)
			{
				return true;
			}

			const FVector ClosestPoint = FMath::ClosestPointOnSegment(WorldPosition, SanctuaryLocation, WaystoneLocation);
			if (FVector::Dist2D(WorldPosition, ClosestPoint) < 330.0f + Radius)
			{
				return true;
			}
		}
		return false;
	}
}
