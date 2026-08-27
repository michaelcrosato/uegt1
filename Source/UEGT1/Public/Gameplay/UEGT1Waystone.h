#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/UEGT1Interactable.h"
#include "UEGT1Waystone.generated.h"

class UPointLightComponent;
class USceneComponent;
class UStaticMesh;
class UStaticMeshComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;

UCLASS()
class UEGT1_API AUEGT1Waystone : public AActor, public IUEGT1Interactable
{
	GENERATED_BODY()

public:
	AUEGT1Waystone();
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "UEGT1|Waystone")
	void InitializeWaystone(FName InWaystoneId);

	UFUNCTION(BlueprintPure, Category = "UEGT1|Waystone")
	FName GetWaystoneId() const { return WaystoneId; }

	UFUNCTION(BlueprintPure, Category = "UEGT1|Waystone")
	bool IsActivated() const { return bActivated; }

	virtual bool CanInteract_Implementation(APawn* InstigatorPawn) const override;
	virtual FText GetInteractionPrompt_Implementation(APawn* InstigatorPawn) const override;
	virtual void Interact_Implementation(APawn* InstigatorPawn) override;
	virtual void SetInteractionFocus_Implementation(bool bNewFocused) override;

protected:
	virtual void BeginPlay() override;

private:
	void ApplyVisualState();
	UMaterialInstanceDynamic* CreateColorMaterial(UPrimitiveComponent* Component, const FLinearColor& Color, bool bGlow = false);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> BaseMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> PedestalMesh;

	UPROPERTY(VisibleAnywhere)
	TArray<TObjectPtr<UStaticMeshComponent>> ShardMeshes;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPointLightComponent> SignalLight;

	UPROPERTY()
	TObjectPtr<UStaticMesh> ConeMesh;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> ShapeMaterial;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> DynamicMaterials;

	UPROPERTY(EditInstanceOnly, Category = "UEGT1|Waystone")
	FName WaystoneId;

	float AnimationTime = 0.0f;
	bool bActivated = false;
	bool bFocused = false;
};
