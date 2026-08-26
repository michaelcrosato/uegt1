#pragma once

#include "CoreMinimal.h"
#include "UEGT1RegionTypes.generated.h"

UENUM(BlueprintType)
enum class EUEGT1RegionBiome : uint8
{
	Town,
	Meadow,
	Farmland,
	Highlands,
	Tropical,
	Coast,
	Ocean,
	Count UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FUEGT1BiomeWeights
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UEGT1|Region") float Town = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UEGT1|Region") float Meadow = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UEGT1|Region") float Farmland = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UEGT1|Region") float Highlands = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UEGT1|Region") float Tropical = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UEGT1|Region") float Coast = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UEGT1|Region") float Ocean = 0.0f;

	float Get(EUEGT1RegionBiome Biome) const;
	void Set(EUEGT1RegionBiome Biome, float Value);
	void Normalize();
	EUEGT1RegionBiome GetDominantBiome() const;
	FString ToCompactString() const;
};

USTRUCT(BlueprintType)
struct FUEGT1RegionSample
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UEGT1|Region") FUEGT1BiomeWeights Biomes;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UEGT1|Region") float SurfaceHeight = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UEGT1|Region") float Temperature = 0.5f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UEGT1|Region") float Moisture = 0.5f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UEGT1|Region") float WaterDepth = 0.0f;

	EUEGT1RegionBiome GetDominantBiome() const { return Biomes.GetDominantBiome(); }
};

UEGT1_API const TCHAR* LexToString(EUEGT1RegionBiome Biome);
