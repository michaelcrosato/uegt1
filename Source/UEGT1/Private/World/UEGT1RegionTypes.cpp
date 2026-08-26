#include "World/UEGT1RegionTypes.h"

float FUEGT1BiomeWeights::Get(EUEGT1RegionBiome Biome) const
{
	switch (Biome)
	{
	case EUEGT1RegionBiome::Town: return Town;
	case EUEGT1RegionBiome::Meadow: return Meadow;
	case EUEGT1RegionBiome::Farmland: return Farmland;
	case EUEGT1RegionBiome::Highlands: return Highlands;
	case EUEGT1RegionBiome::Tropical: return Tropical;
	case EUEGT1RegionBiome::Coast: return Coast;
	case EUEGT1RegionBiome::Ocean: return Ocean;
	default: return 0.0f;
	}
}

void FUEGT1BiomeWeights::Set(EUEGT1RegionBiome Biome, float Value)
{
	switch (Biome)
	{
	case EUEGT1RegionBiome::Town: Town = Value; break;
	case EUEGT1RegionBiome::Meadow: Meadow = Value; break;
	case EUEGT1RegionBiome::Farmland: Farmland = Value; break;
	case EUEGT1RegionBiome::Highlands: Highlands = Value; break;
	case EUEGT1RegionBiome::Tropical: Tropical = Value; break;
	case EUEGT1RegionBiome::Coast: Coast = Value; break;
	case EUEGT1RegionBiome::Ocean: Ocean = Value; break;
	default: break;
	}
}

void FUEGT1BiomeWeights::Normalize()
{
	float Total = 0.0f;
	for (uint8 Index = 0; Index < static_cast<uint8>(EUEGT1RegionBiome::Count); ++Index)
	{
		Total += Get(static_cast<EUEGT1RegionBiome>(Index));
	}
	if (Total <= UE_SMALL_NUMBER)
	{
		Meadow = 1.0f;
		return;
	}
	for (uint8 Index = 0; Index < static_cast<uint8>(EUEGT1RegionBiome::Count); ++Index)
	{
		const EUEGT1RegionBiome Biome = static_cast<EUEGT1RegionBiome>(Index);
		Set(Biome, Get(Biome) / Total);
	}
}

EUEGT1RegionBiome FUEGT1BiomeWeights::GetDominantBiome() const
{
	EUEGT1RegionBiome Dominant = EUEGT1RegionBiome::Meadow;
	float Highest = -1.0f;
	for (uint8 Index = 0; Index < static_cast<uint8>(EUEGT1RegionBiome::Count); ++Index)
	{
		const EUEGT1RegionBiome Candidate = static_cast<EUEGT1RegionBiome>(Index);
		if (Get(Candidate) > Highest)
		{
			Dominant = Candidate;
			Highest = Get(Candidate);
		}
	}
	return Dominant;
}

FString FUEGT1BiomeWeights::ToCompactString() const
{
	return FString::Printf(TEXT("Town=%.2f Meadow=%.2f Farm=%.2f High=%.2f Tropic=%.2f Coast=%.2f Ocean=%.2f"),
		Town, Meadow, Farmland, Highlands, Tropical, Coast, Ocean);
}

const TCHAR* LexToString(EUEGT1RegionBiome Biome)
{
	switch (Biome)
	{
	case EUEGT1RegionBiome::Town: return TEXT("Town");
	case EUEGT1RegionBiome::Meadow: return TEXT("Meadow");
	case EUEGT1RegionBiome::Farmland: return TEXT("Farmland");
	case EUEGT1RegionBiome::Highlands: return TEXT("Highlands");
	case EUEGT1RegionBiome::Tropical: return TEXT("Tropical");
	case EUEGT1RegionBiome::Coast: return TEXT("Coast");
	case EUEGT1RegionBiome::Ocean: return TEXT("Ocean");
	default: return TEXT("Unknown");
	}
}
