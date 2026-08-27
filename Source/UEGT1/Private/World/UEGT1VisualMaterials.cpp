#include "World/UEGT1VisualMaterials.h"

#include "Components/PrimitiveComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/UObjectGlobals.h"

namespace
{
	const TCHAR* GetMaterialPath(EUEGT1VisualMaterial Style)
	{
		switch (Style)
		{
		case EUEGT1VisualMaterial::TechTerrain:
			return TEXT("/Game/Materials/M_TechTerrain.M_TechTerrain");
		case EUEGT1VisualMaterial::TechSurface:
			return TEXT("/Game/Materials/M_TechSurface.M_TechSurface");
		case EUEGT1VisualMaterial::TechFoliage:
			return TEXT("/Game/Materials/M_TechFoliage.M_TechFoliage");
		case EUEGT1VisualMaterial::TechWater:
			return TEXT("/Game/Materials/M_TechWater.M_TechWater");
		case EUEGT1VisualMaterial::Water:
			return TEXT("/Game/Materials/M_SignalWater.M_SignalWater");
		case EUEGT1VisualMaterial::Glow:
			return TEXT("/Game/Materials/M_SignalGlow.M_SignalGlow");
		case EUEGT1VisualMaterial::Surface:
		default:
			return TEXT("/Game/Materials/M_SignalSurface.M_SignalSurface");
		}
	}
}

UMaterialInstanceDynamic* UEGT1VisualMaterials::Apply(
	UObject* Outer,
	UPrimitiveComponent* Component,
	UMaterialInterface* FallbackMaterial,
	EUEGT1VisualMaterial Style,
	const FLinearColor& Color,
	float Roughness,
	float Specular,
	float Metallic,
	float EmissiveStrength)
{
	if (!Outer || !Component)
	{
		return nullptr;
	}

	UMaterialInterface* ParentMaterial = LoadObject<UMaterialInterface>(nullptr, GetMaterialPath(Style));
	if (!ParentMaterial)
	{
		ParentMaterial = FallbackMaterial;
	}
	if (!ParentMaterial)
	{
		return nullptr;
	}

	UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(ParentMaterial, Outer);
	Material->SetVectorParameterValue(TEXT("Color"), Color);
	Material->SetScalarParameterValue(TEXT("Roughness"), Roughness);
	Material->SetScalarParameterValue(TEXT("Specular"), Specular);
	Material->SetScalarParameterValue(TEXT("Metallic"), Metallic);
	Material->SetScalarParameterValue(TEXT("EmissiveStrength"), EmissiveStrength);
	Component->SetMaterial(0, Material);
	return Material;
}
