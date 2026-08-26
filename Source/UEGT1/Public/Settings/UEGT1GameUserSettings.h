#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "UEGT1GameUserSettings.generated.h"

UENUM(BlueprintType)
enum class EUEGT1GraphicsFeature : uint8
{
	AntiAliasing,
	Shadows,
	GlobalIllumination,
	Reflections,
	AmbientOcclusion,
	Bloom,
	MotionBlur,
	DepthOfField,
	LensFlares,
	Fog,
	Foliage,
	Count UMETA(Hidden)
};

DECLARE_MULTICAST_DELEGATE(FUEGT1GraphicsSettingsApplied);

UCLASS(Config = GameUserSettings)
class UEGT1_API UUEGT1GameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:
	static UUEGT1GameUserSettings* Get();
	static FUEGT1GraphicsSettingsApplied OnGraphicsSettingsApplied;

	virtual void ApplyNonResolutionSettings() override;
	virtual void SetToDefaults() override;

	UFUNCTION(BlueprintPure, Category = "UEGT1|Settings")
	bool IsFeatureEnabled(EUEGT1GraphicsFeature Feature) const;

	UFUNCTION(BlueprintCallable, Category = "UEGT1|Settings")
	void SetFeatureEnabled(EUEGT1GraphicsFeature Feature, bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "UEGT1|Settings")
	void SetAllOptionalFeaturesEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "UEGT1|Settings")
	bool AreAllOptionalFeaturesEnabled() const;

	UFUNCTION(BlueprintPure, Category = "UEGT1|Settings")
	bool AreAnyOptionalFeaturesEnabled() const;

	UFUNCTION(BlueprintCallable, Category = "UEGT1|Settings")
	void SetRecommendedDefaults();

	static FText GetFeatureDisplayName(EUEGT1GraphicsFeature Feature);

private:
	void ApplyFeatureCVars() const;

	UPROPERTY(Config)
	bool bAntiAliasingEnabled = true;

	UPROPERTY(Config)
	bool bShadowsEnabled = true;

	UPROPERTY(Config)
	bool bGlobalIlluminationEnabled = true;

	UPROPERTY(Config)
	bool bReflectionsEnabled = true;

	UPROPERTY(Config)
	bool bAmbientOcclusionEnabled = true;

	UPROPERTY(Config)
	bool bBloomEnabled = true;

	UPROPERTY(Config)
	bool bMotionBlurEnabled = true;

	UPROPERTY(Config)
	bool bDepthOfFieldEnabled = true;

	UPROPERTY(Config)
	bool bLensFlaresEnabled = true;

	UPROPERTY(Config)
	bool bFogEnabled = true;

	UPROPERTY(Config)
	bool bFoliageEnabled = true;
};
