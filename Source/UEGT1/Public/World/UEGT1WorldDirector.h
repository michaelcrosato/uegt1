#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UEGT1WorldDirector.generated.h"

UCLASS()
class UEGT1_API AUEGT1WorldDirector : public AActor
{
	GENERATED_BODY()

public:
	AUEGT1WorldDirector();

protected:
	virtual void BeginPlay() override;

private:
	void EnsureBiomeTiles();
	void EnsureGameplayActors();
};
