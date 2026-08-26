#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UEGT1Interactable.generated.h"

UINTERFACE(BlueprintType)
class UEGT1_API UUEGT1Interactable : public UInterface
{
	GENERATED_BODY()
};

class UEGT1_API IUEGT1Interactable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool CanInteract(APawn* InstigatorPawn) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	FText GetInteractionPrompt(APawn* InstigatorPawn) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void Interact(APawn* InstigatorPawn);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void SetInteractionFocus(bool bFocused);
};
