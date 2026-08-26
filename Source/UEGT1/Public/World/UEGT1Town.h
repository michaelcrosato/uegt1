#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UEGT1Town.generated.h"

class UHierarchicalInstancedStaticMeshComponent;
class UMaterialInterface;
class USceneComponent;
class UStaticMesh;

UCLASS()
class UEGT1_API AUEGT1Town : public AActor
{
	GENERATED_BODY()

public:
	AUEGT1Town();
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintPure, Category = "UEGT1|World")
	int32 GetBuildingCount() const { return BuildingCount; }

	UFUNCTION(BlueprintPure, Category = "UEGT1|World")
	int32 GetGeneratedInstanceCount() const;

	UPROPERTY(EditAnywhere, Category = "UEGT1|World", meta = (ClampMin = "1"))
	int32 TownSeedOffset = 401;

private:
	void RebuildTown();
	void AddBuilding(FRandomStream& Random, const FVector& Position);
	void AddRoadsAndPlaza();
	void AddWaterfront();
	void AddStreetFurniture();
	UHierarchicalInstancedStaticMeshComponent* CreateInstanceComponent(const FName Name, UStaticMesh* Mesh, bool bCollision);
	void AssignColorMaterial(UHierarchicalInstancedStaticMeshComponent* Component, const FLinearColor& Color);

	UPROPERTY(VisibleAnywhere) TObjectPtr<USceneComponent> SceneRoot;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> RoadInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> WarmWallInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> CoolWallInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> RoofInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> TrimInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> PierInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> PostInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> LampInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> LighthouseInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> LighthouseRoofInstances;

	UPROPERTY() TObjectPtr<UMaterialInterface> ShapeMaterial;
	int32 BuildingCount = 0;
};
