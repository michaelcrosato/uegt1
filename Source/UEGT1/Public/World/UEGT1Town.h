#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "World/UEGT1TownGeneration.h"
#include "UEGT1Town.generated.h"

class UHierarchicalInstancedStaticMeshComponent;
class UMaterialInterface;
class USceneComponent;
class UStaticMesh;
class UTextRenderComponent;
class UPointLightComponent;
class UUEGT1TownDestinationComponent;
class AUEGT1TownActivityStation;
enum class EUEGT1VisualMaterial : uint8;

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

	UFUNCTION(BlueprintPure, Category = "UEGT1|World")
	int32 GetBedCount() const;

	void RegenerateFromSimulationSeed(int32 Seed);

	UPROPERTY(EditAnywhere, Category = "UEGT1|World", meta = (ClampMin = "1"))
	int32 TownSeedOffset = 401;

private:
	void RebuildTown(int32 Seed);
	void CreateDestinationComponents();
	void CreateActivityStations();
	void AddBuilding(const FUEGT1GeneratedTownLot& Lot, int32 BuildingIndex);
	void AddBeds(const FUEGT1GeneratedTownLot& Lot);
	void AddRoadsAndPlaza(const FUEGT1GeneratedTownLayout& Layout);
	void AddWaterfront();
	void AddStreetFurniture();
	UHierarchicalInstancedStaticMeshComponent* CreateInstanceComponent(const FName Name, UStaticMesh* Mesh, bool bCollision);
	void AssignColorMaterial(UHierarchicalInstancedStaticMeshComponent* Component, const FLinearColor& Color,
		float Roughness, float Specular, EUEGT1VisualMaterial Style, float Metallic = 0.0f, float EmissiveStrength = 0.0f);

	UPROPERTY(VisibleAnywhere) TObjectPtr<USceneComponent> SceneRoot;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> RoadInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> SidewalkInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> TownGrassInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> WarmWallInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> CoolWallInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> RoofInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> TrimInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> InteriorFloorInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> InteriorWallInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> KitchenInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> BathroomInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> ShowerInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> WorkstationInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> BedFrameInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> BeddingInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> PierInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> PostInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> LampInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> LighthouseInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> LighthouseRoofInstances;

	UPROPERTY() TObjectPtr<UMaterialInterface> ShapeMaterial;
	UPROPERTY(Transient) TArray<TObjectPtr<UUEGT1TownDestinationComponent>> DestinationComponents;
	UPROPERTY(Transient) TArray<TObjectPtr<AUEGT1TownActivityStation>> ActivityStations;
	UPROPERTY(Transient) TArray<TObjectPtr<UTextRenderComponent>> BuildingSigns;
	UPROPERTY(Transient) TArray<TObjectPtr<UPointLightComponent>> InteriorLights;
	FUEGT1GeneratedTownLayout GeneratedLayout;
	int32 BuildingCount = 0;
};
