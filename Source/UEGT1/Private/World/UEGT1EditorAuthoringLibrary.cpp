#include "World/UEGT1EditorAuthoringLibrary.h"

#include "Engine/Level.h"
#include "Engine/World.h"

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
