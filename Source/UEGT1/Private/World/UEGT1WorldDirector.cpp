#include "World/UEGT1WorldDirector.h"

#include "EngineUtils.h"
#include "Gameplay/UEGT1Sanctuary.h"
#include "Gameplay/UEGT1Waystone.h"
#include "Kismet/GameplayStatics.h"
#include "UEGT1LogChannels.h"
#include "World/UEGT1BiomeTile.h"
#include "World/UEGT1WorldLayout.h"

AUEGT1WorldDirector::AUEGT1WorldDirector()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AUEGT1WorldDirector::BeginPlay()
{
	Super::BeginPlay();
	EnsureBiomeTiles();
	EnsureGameplayActors();

	int32 TileCount = 0;
	int32 InstanceCount = 0;
	for (TActorIterator<AUEGT1BiomeTile> Iterator(GetWorld()); Iterator; ++Iterator)
	{
		++TileCount;
		InstanceCount += Iterator->GetGeneratedInstanceCount();
	}
	UE_LOG(LogUEGT1, Display, TEXT("Signal Grove ready: Seed=%d Tiles=%d Instances=%d Waystones=%d"),
		UEGT1WorldLayout::WorldSeed, TileCount, InstanceCount, UEGT1WorldLayout::GetWaystoneLocations().Num());
}

void AUEGT1WorldDirector::EnsureBiomeTiles()
{
	for (TActorIterator<AUEGT1BiomeTile> Iterator(GetWorld()); Iterator; ++Iterator)
	{
		return;
	}

	UE_LOG(LogUEGT1, Warning, TEXT("No authored biome tiles found; generating runtime fallback tiles."));
	for (int32 Y = -UEGT1WorldLayout::TileRadius; Y <= UEGT1WorldLayout::TileRadius; ++Y)
	{
		for (int32 X = -UEGT1WorldLayout::TileRadius; X <= UEGT1WorldLayout::TileRadius; ++X)
		{
			AUEGT1BiomeTile* Tile = GetWorld()->SpawnActor<AUEGT1BiomeTile>(AUEGT1BiomeTile::StaticClass(), FTransform::Identity);
			Tile->InitializeTile(FIntPoint(X, Y), UEGT1WorldLayout::WorldSeed);
		}
	}
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
		GetWorld()->SpawnActor<AUEGT1Sanctuary>(AUEGT1Sanctuary::StaticClass(), UEGT1WorldLayout::SanctuaryLocation, FRotator::ZeroRotator);
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

		const FTransform SpawnTransform((UEGT1WorldLayout::SanctuaryLocation - Locations[Index]).Rotation(), Locations[Index]);
		AUEGT1Waystone* Waystone = GetWorld()->SpawnActorDeferred<AUEGT1Waystone>(AUEGT1Waystone::StaticClass(), SpawnTransform, this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		Waystone->InitializeWaystone(Ids[Index]);
		UGameplayStatics::FinishSpawningActor(Waystone, SpawnTransform);
	}
}
