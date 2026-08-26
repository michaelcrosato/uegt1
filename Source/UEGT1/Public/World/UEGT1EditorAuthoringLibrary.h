#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UEGT1EditorAuthoringLibrary.generated.h"

UCLASS()
class UEGT1_API UUEGT1EditorAuthoringLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "UEGT1|Editor")
	static bool ConvertLevelActorsToExternalPackages(UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "UEGT1|Editor")
	static int32 GetRegionWorldSeed();

	UFUNCTION(BlueprintPure, Category = "UEGT1|Editor")
	static int32 GetRegionTileRadius();

	UFUNCTION(BlueprintPure, Category = "UEGT1|Editor")
	static float GetRegionTileSize();

	UFUNCTION(BlueprintPure, Category = "UEGT1|Editor")
	static TArray<FVector> GetRegionWaystoneLocations();

	UFUNCTION(BlueprintPure, Category = "UEGT1|Editor")
	static TArray<FName> GetRegionWaystoneIds();

	UFUNCTION(BlueprintPure, Category = "UEGT1|Editor")
	static float GetRegionSurfaceHeight(FVector WorldPosition);
};
