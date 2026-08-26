#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UEGT1Sanctuary.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class UPointLightComponent;
class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class UEGT1_API AUEGT1Sanctuary : public AActor
{
	GENERATED_BODY()

public:
	AUEGT1Sanctuary();
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintPure, Category = "UEGT1|Sanctuary")
	bool IsRestored() const { return bRestored; }

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleObjectiveProgress(int32 ActivatedCount, int32 TotalCount, bool bComplete);

	void ApplyVisualState();
	UMaterialInstanceDynamic* CreateColorMaterial(UPrimitiveComponent* Component, const FLinearColor& Color);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> MotionRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> PlatformMesh;

	UPROPERTY(VisibleAnywhere)
	TArray<TObjectPtr<UStaticMeshComponent>> OrbitingSlabs;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> CoreMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPointLightComponent> CoreLight;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> ShapeMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> CoreMaterial;

	float AnimationTime = 0.0f;
	bool bRestored = false;
};
