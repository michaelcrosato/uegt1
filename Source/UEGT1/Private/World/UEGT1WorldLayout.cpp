#include "World/UEGT1WorldLayout.h"

#include "World/UEGT1RegionSettings.h"

namespace
{
	float SmoothRange(float Start, float End, float Value)
	{
		const float Alpha = FMath::Clamp((Value - Start) / FMath::Max(End - Start, 1.0f), 0.0f, 1.0f);
		return Alpha * Alpha * (3.0f - 2.0f * Alpha);
	}

	float HashNoise(int32 X, int32 Y, int32 Seed)
	{
		uint32 Hash = HashCombineFast(GetTypeHash(X), GetTypeHash(Y));
		Hash = HashCombineFast(Hash, GetTypeHash(Seed));
		Hash ^= Hash >> 16;
		return (static_cast<float>(Hash & 0xffffu) / 32767.5f) - 1.0f;
	}

	float ValueNoise(const FVector& Position, float CellSize, int32 Seed)
	{
		const float GridX = Position.X / CellSize;
		const float GridY = Position.Y / CellSize;
		const int32 X0 = FMath::FloorToInt(GridX);
		const int32 Y0 = FMath::FloorToInt(GridY);
		const float AlphaX = SmoothRange(0.0f, 1.0f, GridX - X0);
		const float AlphaY = SmoothRange(0.0f, 1.0f, GridY - Y0);
		const float A = FMath::Lerp(HashNoise(X0, Y0, Seed), HashNoise(X0 + 1, Y0, Seed), AlphaX);
		const float B = FMath::Lerp(HashNoise(X0, Y0 + 1, Seed), HashNoise(X0 + 1, Y0 + 1, Seed), AlphaX);
		return FMath::Lerp(A, B, AlphaY);
	}
}

namespace UEGT1WorldLayout
{
	int32 GetWorldSeed() { return UUEGT1RegionSettings::Get().WorldSeed; }
	int32 GetTileRadius() { return UUEGT1RegionSettings::Get().TileRadius; }
	int32 GetExpectedTileCount() { const int32 Diameter = GetTileRadius() * 2 + 1; return Diameter * Diameter; }
	float GetTileSize() { return UUEGT1RegionSettings::Get().TileSize; }
	float GetWorldHalfExtent() { return (GetTileRadius() + 0.5f) * GetTileSize(); }
	float GetSeaLevel() { return UUEGT1RegionSettings::Get().SeaLevel; }

	const FVector& GetSanctuaryLocation()
	{
		static const FVector Location(0.0f, 0.0f, 0.0f);
		return Location;
	}

	const FVector& GetPlayerStartLocation()
	{
		static const FVector Location(0.0f, -1350.0f, 110.0f);
		return Location;
	}

	const TArray<FVector>& GetWaystoneLocations()
	{
		static const TArray<FVector> Locations = {
			FVector(7200.0f, 3200.0f, 0.0f),
			FVector(-8500.0f, 2800.0f, 0.0f),
			FVector(900.0f, -8500.0f, 0.0f)
		};
		return Locations;
	}

	const TArray<FName>& GetWaystoneIds()
	{
		static const TArray<FName> Ids = { TEXT("EastRise"), TEXT("WestHollow"), TEXT("SouthWatch") };
		return Ids;
	}

	FUEGT1RegionSample SampleRegion(const FVector& WorldPosition)
	{
		const UUEGT1RegionSettings& Settings = UUEGT1RegionSettings::Get();
		const float Radius = FVector2D(WorldPosition).Size();
		const float TownWeight = 1.0f - SmoothRange(Settings.TownCoreRadius, Settings.TownBlendRadius, Radius);
		const float NonTown = 1.0f - TownWeight;
		const float Farmland = SmoothRange(Settings.DirectionalBiomeStart, Settings.DirectionalBiomeFull, -WorldPosition.X);
		const float Highlands = SmoothRange(Settings.DirectionalBiomeStart, Settings.DirectionalBiomeFull, WorldPosition.Y);
		const float Tropical = SmoothRange(Settings.DirectionalBiomeStart, Settings.DirectionalBiomeFull, -WorldPosition.Y);
		const float Ocean = SmoothRange(Settings.OceanStart, Settings.OceanFull, WorldPosition.X);
		const float Coast = SmoothRange(Settings.CoastStart, Settings.OceanStart, WorldPosition.X) * (1.0f - Ocean);
		const float StrongestDirection = FMath::Max(FMath::Max(Farmland, Highlands), FMath::Max(Tropical, FMath::Max(Coast, Ocean)));

		FUEGT1RegionSample Sample;
		Sample.Biomes.Town = TownWeight;
		Sample.Biomes.Meadow = NonTown * (1.0f - 0.85f * StrongestDirection);
		Sample.Biomes.Farmland = NonTown * Farmland;
		Sample.Biomes.Highlands = NonTown * Highlands;
		Sample.Biomes.Tropical = NonTown * Tropical;
		Sample.Biomes.Coast = NonTown * Coast;
		Sample.Biomes.Ocean = NonTown * Ocean;
		Sample.Biomes.Normalize();

		const float BroadNoise = ValueNoise(WorldPosition, 2600.0f, Settings.WorldSeed);
		const float DetailNoise = ValueNoise(WorldPosition, 900.0f, Settings.WorldSeed + 101);
		const float NorthProgress = SmoothRange(Settings.DirectionalBiomeStart, Settings.DirectionalBiomeFull, WorldPosition.Y);
		const float OceanProgress = SmoothRange(Settings.CoastStart, Settings.OceanFull, WorldPosition.X);
		const float MeadowHeight = BroadNoise * 75.0f + DetailNoise * 28.0f;
		const float FarmHeight = BroadNoise * 95.0f + FMath::Sin(WorldPosition.Y * 0.00065f) * 34.0f;
		const float HighlandHeight = 180.0f + NorthProgress * Settings.MountainMaxElevation + FMath::Abs(BroadNoise) * 560.0f + DetailNoise * 110.0f;
		const float TropicalHeight = 45.0f + BroadNoise * 125.0f + DetailNoise * 55.0f;
		const float CoastHeight = FMath::Lerp(25.0f, Settings.SeaLevel - 110.0f, OceanProgress) + DetailNoise * 26.0f;
		const float OceanHeight = Settings.SeaLevel - 260.0f - OceanProgress * 520.0f + BroadNoise * 70.0f;
		Sample.SurfaceHeight = Sample.Biomes.Meadow * MeadowHeight + Sample.Biomes.Farmland * FarmHeight +
			Sample.Biomes.Highlands * HighlandHeight + Sample.Biomes.Tropical * TropicalHeight +
			Sample.Biomes.Coast * CoastHeight + Sample.Biomes.Ocean * OceanHeight;
		Sample.SurfaceHeight *= 1.0f - Sample.Biomes.Town;
		Sample.WaterDepth = FMath::Max(0.0f, Settings.SeaLevel - Sample.SurfaceHeight);
		Sample.Temperature = FMath::Clamp(0.48f + Sample.Biomes.Tropical * 0.45f + Sample.Biomes.Ocean * 0.08f - Sample.Biomes.Highlands * 0.42f, 0.0f, 1.0f);
		Sample.Moisture = FMath::Clamp(0.42f + Sample.Biomes.Tropical * 0.45f + Sample.Biomes.Coast * 0.35f + Sample.Biomes.Ocean * 0.55f - Sample.Biomes.Farmland * 0.12f, 0.0f, 1.0f);
		return Sample;
	}

	bool IsPrimaryRoute(const FVector& WorldPosition, float Radius)
	{
		for (const FVector& WaystoneLocation : GetWaystoneLocations())
		{
			if (FVector::Dist2D(WorldPosition, WaystoneLocation) < 650.0f + Radius)
			{
				return true;
			}
			const FVector ClosestPoint = FMath::ClosestPointOnSegment(WorldPosition, GetSanctuaryLocation(), WaystoneLocation);
			if (FVector::Dist2D(WorldPosition, ClosestPoint) < 360.0f + Radius)
			{
				return true;
			}
		}
		return false;
	}

	bool IsReservedGameplaySpace(const FVector& WorldPosition, float Radius)
	{
		if (FVector::Dist2D(WorldPosition, GetSanctuaryLocation()) < UUEGT1RegionSettings::Get().TownReserveRadius + Radius)
		{
			return true;
		}
		return IsPrimaryRoute(WorldPosition, Radius);
	}
}
