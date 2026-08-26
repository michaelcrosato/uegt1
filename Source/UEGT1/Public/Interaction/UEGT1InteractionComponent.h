#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UEGT1InteractionComponent.generated.h"

UCLASS(ClassGroup = (UEGT1), meta = (BlueprintSpawnableComponent))
class UEGT1_API UUEGT1InteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UUEGT1InteractionComponent();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void TryInteract();

	UFUNCTION(BlueprintPure, Category = "Interaction")
	AActor* GetFocusedActor() const { return FocusedActor.Get(); }

	UFUNCTION(BlueprintPure, Category = "Interaction")
	FText GetFocusedPrompt() const;

	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool HasValidFocus() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", meta = (ClampMin = "100.0", UIMin = "100.0"))
	float TraceDistance = 500.0f;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void RefreshFocus();
	void SetFocusedActor(AActor* NewFocusedActor);

	TWeakObjectPtr<AActor> FocusedActor;
};
