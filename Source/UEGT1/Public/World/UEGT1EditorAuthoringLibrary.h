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
};
