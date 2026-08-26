#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "World/UEGT1RegionTypes.h"
#include "UEGT1BiomeTile.generated.h"

class UHierarchicalInstancedStaticMeshComponent;
class UMaterialInterface;
class USceneComponent;
class UStaticMesh;

UCLASS()
class UEGT1_API AUEGT1BiomeTile : public AActor
{
	GENERATED_BODY()

public:
	AUEGT1BiomeTile();
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "UEGT1|World")
	void InitializeTile(FIntPoint InTileCoordinate, int32 InWorldSeed);

	UFUNCTION(BlueprintPure, Category = "UEGT1|World")
	FIntPoint GetTileCoordinate() const { return TileCoordinate; }

	UFUNCTION(BlueprintPure, Category = "UEGT1|World")
	EUEGT1RegionBiome GetDominantBiome() const { return DominantBiome; }

	UFUNCTION(BlueprintPure, Category = "UEGT1|World")
	int32 GetGeneratedInstanceCount() const;

	UPROPERTY(EditInstanceOnly, Category = "UEGT1|World")
	FIntPoint TileCoordinate = FIntPoint::ZeroValue;

	UPROPERTY(EditInstanceOnly, Category = "UEGT1|World")
	int32 WorldSeed = 7319;

	UPROPERTY(EditDefaultsOnly, Category = "UEGT1|World", meta = (ClampMin = "800.0"))
	float TileSize = 3200.0f;

	UPROPERTY(EditDefaultsOnly, Category = "UEGT1|World", meta = (ClampMin = "3", ClampMax = "12"))
	int32 TerrainCellsPerAxis = 6;

	UPROPERTY(EditDefaultsOnly, Category = "UEGT1|World", meta = (ClampMin = "0"))
	int32 TreesPerTile = 14;

	UPROPERTY(EditDefaultsOnly, Category = "UEGT1|World", meta = (ClampMin = "0"))
	int32 RocksPerTile = 7;

	UPROPERTY(EditDefaultsOnly, Category = "UEGT1|World", meta = (ClampMin = "0"))
	int32 GrassClustersPerTile = 18;

private:
	void RebuildTile();
	void GenerateTerrain(FRandomStream& Random);
	void GenerateRoutes(FRandomStream& Random);
	void GenerateVegetation(FRandomStream& Random);
	void GenerateRegionalFeatures(FRandomStream& Random);
	void ApplyGraphicsSettings();
	FLinearColor GetBlendedGroundColor(const FUEGT1BiomeWeights& Weights) const;
	UHierarchicalInstancedStaticMeshComponent* CreateInstanceComponent(const FName Name, UStaticMesh* Mesh, bool bCollision);
	void AssignColorMaterial(UHierarchicalInstancedStaticMeshComponent* Component, const FLinearColor& Color);
	bool IsInsideTile(const FVector& WorldPosition, float Padding = 0.0f) const;

	UPROPERTY(VisibleAnywhere) TObjectPtr<USceneComponent> SceneRoot;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> GroundInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> TrailInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> TrunkInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> ConiferInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> BroadleafInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> RockInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> GrassInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> CropInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> WaterInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> WaveInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> PeakInstances;

	UPROPERTY() TObjectPtr<UMaterialInterface> ShapeMaterial;
	EUEGT1RegionBiome DominantBiome = EUEGT1RegionBiome::Meadow;
};
