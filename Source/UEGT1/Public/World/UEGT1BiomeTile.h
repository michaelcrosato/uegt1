#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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

	UFUNCTION(BlueprintCallable, Category = "UEGT1|World")
	void InitializeTile(FIntPoint InTileCoordinate, int32 InWorldSeed);

	UFUNCTION(BlueprintPure, Category = "UEGT1|World")
	FIntPoint GetTileCoordinate() const { return TileCoordinate; }

	UFUNCTION(BlueprintPure, Category = "UEGT1|World")
	int32 GetGeneratedInstanceCount() const;

	UPROPERTY(EditInstanceOnly, Category = "UEGT1|World")
	FIntPoint TileCoordinate = FIntPoint::ZeroValue;

	UPROPERTY(EditInstanceOnly, Category = "UEGT1|World")
	int32 WorldSeed = 7319;

	UPROPERTY(EditDefaultsOnly, Category = "UEGT1|World", meta = (ClampMin = "800.0"))
	float TileSize = 3200.0f;

	UPROPERTY(EditDefaultsOnly, Category = "UEGT1|World", meta = (ClampMin = "0"))
	int32 TreesPerTile = 14;

	UPROPERTY(EditDefaultsOnly, Category = "UEGT1|World", meta = (ClampMin = "0"))
	int32 RocksPerTile = 7;

	UPROPERTY(EditDefaultsOnly, Category = "UEGT1|World", meta = (ClampMin = "0"))
	int32 GrassClustersPerTile = 18;

private:
	void RebuildTile();
	void GenerateGroundAndTrails(FRandomStream& Random);
	void GenerateVegetation(FRandomStream& Random);
	void GenerateBoundaryCliffs(FRandomStream& Random);
	UHierarchicalInstancedStaticMeshComponent* CreateInstanceComponent(const FName Name, UStaticMesh* Mesh, bool bCollision);
	void AssignColorMaterial(UHierarchicalInstancedStaticMeshComponent* Component, const FLinearColor& Color);
	bool IsInsideTile(const FVector& WorldPosition, float Padding = 0.0f) const;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> GroundInstances;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> TrailInstances;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> TrunkInstances;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> CanopyInstances;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> RockInstances;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> GrassInstances;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> ShapeMaterial;
};
