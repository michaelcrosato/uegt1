#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UEGT1RegionSettings.generated.h"

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Signal Grove Region"))
class UEGT1_API UUEGT1RegionSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	static const UUEGT1RegionSettings& Get();

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Layout", meta = (ClampMin = "1"))
	int32 WorldSeed = 7319;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Layout", meta = (ClampMin = "2", ClampMax = "16"))
	int32 TileRadius = 5;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Layout", meta = (ClampMin = "0", ClampMax = "16"))
	int32 WestTileExtension = 3;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Layout", meta = (ClampMin = "800.0"))
	float TileSize = 3200.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Town", meta = (ClampMin = "500.0"))
	float TownCoreRadius = 3200.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Town", meta = (ClampMin = "1000.0"))
	float TownBlendRadius = 6200.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Town", meta = (ClampMin = "1000.0"))
	float TownReserveRadius = 5600.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Town", meta = (ClampMin = "0.0"))
	float TownWestExtension = 13000.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Biomes", meta = (ClampMin = "0.0"))
	float DirectionalBiomeStart = 2800.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Biomes", meta = (ClampMin = "1000.0"))
	float DirectionalBiomeFull = 12600.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Waterfront", meta = (ClampMin = "0.0"))
	float CoastStart = 3600.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Waterfront", meta = (ClampMin = "1000.0"))
	float OceanStart = 7800.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Waterfront", meta = (ClampMin = "1000.0"))
	float OceanFull = 10800.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Waterfront")
	float SeaLevel = -80.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Mountains", meta = (ClampMin = "500.0"))
	float MountainMaxElevation = 2600.0f;
};
