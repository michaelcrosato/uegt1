#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UEGT1TownResident.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class USceneComponent;
class UStaticMeshComponent;

struct UEGT1_API FUEGT1ResidentVisualMotion
{
	void Snap(const FVector& Position);
	void Retarget(const FVector& CurrentPosition, const FVector& NewTarget, float DurationSeconds);
	FVector Advance(float DeltaSeconds);
	bool IsMoving() const;
	const FVector& GetCurrentPosition() const { return CurrentPosition; }
	const FVector& GetTargetPosition() const { return TargetPosition; }
	FVector GetFacingDirection() const { return (TargetPosition - StartPosition).GetSafeNormal2D(); }

private:
	FVector StartPosition = FVector::ZeroVector;
	FVector TargetPosition = FVector::ZeroVector;
	FVector CurrentPosition = FVector::ZeroVector;
	float ElapsedSeconds = 0.0f;
	float DurationSeconds = 0.0f;
};

UCLASS(Blueprintable)
class UEGT1_API AUEGT1TownResident final : public AActor
{
	GENERATED_BODY()

public:
	AUEGT1TownResident();
	void InitializeResident(FName InNpcId, int32 VisualSeed);
	void ApplySimulationPosition(const FVector& Position);
	void SetSimulationTarget(const FVector& Position, float BlendDurationSeconds);
	void AdvanceVisualMovement(float DeltaSeconds);
	bool IsVisualMovementActive() const { return VisualMotion.IsMoving(); }
	FVector GetVisualTargetPosition() const { return VisualMotion.GetTargetPosition(); }

	UFUNCTION(BlueprintPure, Category = "Town Simulation")
	FName GetNpcId() const { return NpcId; }

private:
	UPROPERTY(VisibleAnywhere) TObjectPtr<USceneComponent> SceneRoot;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> BodyMesh;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> HeadMesh;
	UPROPERTY() TObjectPtr<UMaterialInterface> ShapeMaterial;
	UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> BodyMaterial;
	UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> HeadMaterial;
	UPROPERTY(VisibleInstanceOnly, Category = "Town Simulation") FName NpcId;
	FUEGT1ResidentVisualMotion VisualMotion;
	FRotator TargetRotation = FRotator::ZeroRotator;
};
