#include "World/UEGT1EditorAuthoringLibrary.h"

#include "Engine/Level.h"
#include "Engine/World.h"
#include "World/UEGT1WorldLayout.h"

bool UUEGT1EditorAuthoringLibrary::ConvertLevelActorsToExternalPackages(UObject* WorldContextObject)
{
#if WITH_EDITOR
	UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	if (!World || !World->PersistentLevel)
	{
		return false;
	}

	World->PersistentLevel->ConvertAllActorsToPackaging(true);
	return true;
#else
	return false;
#endif
}

int32 UUEGT1EditorAuthoringLibrary::GetRegionWorldSeed()
{
	return UEGT1WorldLayout::GetWorldSeed();
}

int32 UUEGT1EditorAuthoringLibrary::GetRegionTileRadius()
{
	return UEGT1WorldLayout::GetTileRadius();
}

int32 UUEGT1EditorAuthoringLibrary::GetRegionMinTileX()
{
	return UEGT1WorldLayout::GetMinTileX();
}

int32 UUEGT1EditorAuthoringLibrary::GetRegionMaxTileX()
{
	return UEGT1WorldLayout::GetMaxTileX();
}

int32 UUEGT1EditorAuthoringLibrary::GetRegionMinTileY()
{
	return UEGT1WorldLayout::GetMinTileY();
}

int32 UUEGT1EditorAuthoringLibrary::GetRegionMaxTileY()
{
	return UEGT1WorldLayout::GetMaxTileY();
}

int32 UUEGT1EditorAuthoringLibrary::GetRegionExpectedTileCount()
{
	return UEGT1WorldLayout::GetExpectedTileCount();
}

float UUEGT1EditorAuthoringLibrary::GetRegionTileSize()
{
	return UEGT1WorldLayout::GetTileSize();
}

TArray<FVector> UUEGT1EditorAuthoringLibrary::GetRegionWaystoneLocations()
{
	return UEGT1WorldLayout::GetWaystoneLocations();
}

TArray<FName> UUEGT1EditorAuthoringLibrary::GetRegionWaystoneIds()
{
	return UEGT1WorldLayout::GetWaystoneIds();
}

float UUEGT1EditorAuthoringLibrary::GetRegionSurfaceHeight(FVector WorldPosition)
{
	return UEGT1WorldLayout::SampleRegion(WorldPosition).SurfaceHeight;
}
