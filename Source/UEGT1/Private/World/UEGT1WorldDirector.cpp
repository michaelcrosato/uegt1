#include "World/UEGT1WorldDirector.h"

#include "EngineUtils.h"
#include "Gameplay/UEGT1Sanctuary.h"
#include "Gameplay/UEGT1Waystone.h"
#include "Kismet/GameplayStatics.h"
#include "UEGT1LogChannels.h"
#include "World/UEGT1BiomeTile.h"
#include "World/UEGT1Town.h"
#include "World/UEGT1WorldLayout.h"

AUEGT1WorldDirector::AUEGT1WorldDirector()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AUEGT1WorldDirector::BeginPlay()
{
	Super::BeginPlay();
	EnsureBiomeTiles();
	EnsureTown();
	EnsureGameplayActors();

	int32 TileCount = 0;
	int32 InstanceCount = 0;
	int32 BiomeCounts[static_cast<uint8>(EUEGT1RegionBiome::Count)] = {};
	for (TActorIterator<AUEGT1BiomeTile> Iterator(GetWorld()); Iterator; ++Iterator)
	{
		++TileCount;
		InstanceCount += Iterator->GetGeneratedInstanceCount();
		++BiomeCounts[static_cast<uint8>(Iterator->GetDominantBiome())];
	}
	int32 TownBuildings = 0;
	for (TActorIterator<AUEGT1Town> Iterator(GetWorld()); Iterator; ++Iterator)
	{
		TownBuildings += Iterator->GetBuildingCount();
		InstanceCount += Iterator->GetGeneratedInstanceCount();
	}
	UE_LOG(LogUEGT1, Display, TEXT("Regional foundation ready: Seed=%d Tiles=%d Expected=%d Instances=%d TownBuildings=%d Biomes=Town:%d Meadow:%d Farmland:%d Highlands:%d Tropical:%d Coast:%d Ocean:%d"),
		UEGT1WorldLayout::GetWorldSeed(), TileCount, UEGT1WorldLayout::GetExpectedTileCount(), InstanceCount, TownBuildings,
		BiomeCounts[static_cast<uint8>(EUEGT1RegionBiome::Town)], BiomeCounts[static_cast<uint8>(EUEGT1RegionBiome::Meadow)],
		BiomeCounts[static_cast<uint8>(EUEGT1RegionBiome::Farmland)], BiomeCounts[static_cast<uint8>(EUEGT1RegionBiome::Highlands)],
		BiomeCounts[static_cast<uint8>(EUEGT1RegionBiome::Tropical)], BiomeCounts[static_cast<uint8>(EUEGT1RegionBiome::Coast)],
		BiomeCounts[static_cast<uint8>(EUEGT1RegionBiome::Ocean)]);
	if (TileCount != UEGT1WorldLayout::GetExpectedTileCount())
	{
		UE_LOG(LogUEGT1, Error, TEXT("Regional tile coverage mismatch: Loaded=%d Expected=%d"), TileCount, UEGT1WorldLayout::GetExpectedTileCount());
	}
}

void AUEGT1WorldDirector::EnsureBiomeTiles()
{
	for (TActorIterator<AUEGT1BiomeTile> Iterator(GetWorld()); Iterator; ++Iterator)
	{
		return;
	}

	UE_LOG(LogUEGT1, Warning, TEXT("No authored biome tiles found; generating runtime fallback tiles."));
	const int32 Radius = UEGT1WorldLayout::GetTileRadius();
	for (int32 Y = -Radius; Y <= Radius; ++Y)
	{
		for (int32 X = -Radius; X <= Radius; ++X)
		{
			AUEGT1BiomeTile* Tile = GetWorld()->SpawnActor<AUEGT1BiomeTile>(AUEGT1BiomeTile::StaticClass(), FTransform::Identity);
			Tile->InitializeTile(FIntPoint(X, Y), UEGT1WorldLayout::GetWorldSeed());
		}
	}
}

void AUEGT1WorldDirector::EnsureTown()
{
	for (TActorIterator<AUEGT1Town> Iterator(GetWorld()); Iterator; ++Iterator)
	{
		return;
	}
	UE_LOG(LogUEGT1, Warning, TEXT("No authored town found; generating runtime fallback town."));
	GetWorld()->SpawnActor<AUEGT1Town>(AUEGT1Town::StaticClass(), UEGT1WorldLayout::GetSanctuaryLocation(), FRotator::ZeroRotator);
}

void AUEGT1WorldDirector::EnsureGameplayActors()
{
	bool bHasSanctuary = false;
	for (TActorIterator<AUEGT1Sanctuary> Iterator(GetWorld()); Iterator; ++Iterator)
	{
		bHasSanctuary = true;
		break;
	}
	if (!bHasSanctuary)
	{
		GetWorld()->SpawnActor<AUEGT1Sanctuary>(AUEGT1Sanctuary::StaticClass(), UEGT1WorldLayout::GetSanctuaryLocation(), FRotator::ZeroRotator);
	}

	TSet<FName> ExistingIds;
	for (TActorIterator<AUEGT1Waystone> Iterator(GetWorld()); Iterator; ++Iterator)
	{
		ExistingIds.Add(Iterator->GetWaystoneId());
	}

	const TArray<FVector>& Locations = UEGT1WorldLayout::GetWaystoneLocations();
	const TArray<FName>& Ids = UEGT1WorldLayout::GetWaystoneIds();
	for (int32 Index = 0; Index < Locations.Num(); ++Index)
	{
		if (ExistingIds.Contains(Ids[Index]))
		{
			continue;
		}

		FVector SpawnLocation = Locations[Index];
		SpawnLocation.Z = UEGT1WorldLayout::SampleRegion(SpawnLocation).SurfaceHeight;
		const FTransform SpawnTransform((UEGT1WorldLayout::GetSanctuaryLocation() - SpawnLocation).Rotation(), SpawnLocation);
		AUEGT1Waystone* Waystone = GetWorld()->SpawnActorDeferred<AUEGT1Waystone>(AUEGT1Waystone::StaticClass(), SpawnTransform, this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		Waystone->InitializeWaystone(Ids[Index]);
		UGameplayStatics::FinishSpawningActor(Waystone, SpawnTransform);
	}
}
