#pragma once

#include "CoreMinimal.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class UPrimitiveComponent;

enum class EUEGT1VisualMaterial : uint8
{
	Surface,
	Water,
	Glow,
	TechTerrain,
	TechSurface,
	TechFoliage,
	TechWater
};

namespace UEGT1VisualMaterials
{
	UEGT1_API UMaterialInstanceDynamic* Apply(
		UObject* Outer,
		UPrimitiveComponent* Component,
		UMaterialInterface* FallbackMaterial,
		EUEGT1VisualMaterial Style,
		const FLinearColor& Color,
		float Roughness,
		float Specular,
		float Metallic = 0.0f,
		float EmissiveStrength = 0.0f);
}
