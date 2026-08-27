#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UEGT1TechDemoEnvironment.generated.h"

class UHierarchicalInstancedStaticMeshComponent;
class UMaterialInterface;
class UPrimitiveComponent;
class UProceduralMeshComponent;
class USceneComponent;
class UStaticMesh;
enum class EUEGT1VisualMaterial : uint8;

UCLASS()
class UEGT1_API AUEGT1TechDemoEnvironment final : public AActor
{
	GENERATED_BODY()

public:
	AUEGT1TechDemoEnvironment();
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintPure, Category = "UEGT1|Tech Demo")
	int32 GetGeneratedInstanceCount() const;

	UFUNCTION(BlueprintPure, Category = "UEGT1|Tech Demo")
	static float SampleTerrainHeight(const FVector2D& Position);

	static FVector GetRecommendedPlayerStart();

	UPROPERTY(EditDefaultsOnly, Category = "UEGT1|Tech Demo", meta = (ClampMin = "16000.0"))
	float TerrainSize = 36000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "UEGT1|Tech Demo", meta = (ClampMin = "65", ClampMax = "257"))
	int32 TerrainResolution = 129;

	UPROPERTY(EditDefaultsOnly, Category = "UEGT1|Tech Demo", meta = (ClampMin = "100"))
	int32 TreeCandidates = 2100;

	UPROPERTY(EditDefaultsOnly, Category = "UEGT1|Tech Demo", meta = (ClampMin = "1000"))
	int32 GroundDetailCandidates = 14500;

private:
	void RebuildEnvironment();
	void GenerateTerrain();
	void GenerateWaterAndPaths(FRandomStream& Random);
	void GenerateForest(FRandomStream& Random);
	void GenerateGroundDetail(FRandomStream& Random);
	void GenerateRockFormations(FRandomStream& Random);
	void ApplyMaterials();
	void ApplyGraphicsSettings();
	bool IsLake(const FVector2D& Position, float Margin = 0.0f) const;
	bool IsTrail(const FVector2D& Position, float Margin = 0.0f) const;
	float SampleSlope(const FVector2D& Position) const;
	UHierarchicalInstancedStaticMeshComponent* CreateInstances(FName Name, UStaticMesh* Mesh, bool bCollision,
		int32 StartCullDistance, int32 EndCullDistance);
	void ApplyMaterial(UPrimitiveComponent* Component, EUEGT1VisualMaterial Style, const FLinearColor& Color,
		float Roughness, float Specular, float Metallic = 0.0f, float EmissiveStrength = 0.0f);

	UPROPERTY(VisibleAnywhere) TObjectPtr<USceneComponent> SceneRoot;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UProceduralMeshComponent> TerrainMesh;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UProceduralMeshComponent> LakeSurface;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> WaterInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> PathInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> TreeInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> TrunkInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> CanopyInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> PineInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> RockInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> CliffInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> GrassInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> FernInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> FlowerInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> FallenLogInstances;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UHierarchicalInstancedStaticMeshComponent> FoamInstances;

	UPROPERTY() TObjectPtr<UMaterialInterface> FallbackMaterial;
	int32 GeneratedTreeCount = 0;
};
