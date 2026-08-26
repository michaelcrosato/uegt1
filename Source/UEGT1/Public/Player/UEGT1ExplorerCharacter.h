#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "UEGT1ExplorerCharacter.generated.h"

class UCameraComponent;
class UUEGT1InteractionComponent;

UCLASS()
class UEGT1_API AUEGT1ExplorerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AUEGT1ExplorerCharacter();
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintPure, Category = "UEGT1|Interaction")
	UUEGT1InteractionComponent* GetInteractionComponent() const { return InteractionComponent; }

	UFUNCTION(BlueprintPure, Category = "UEGT1|Movement")
	bool IsSprinting() const { return bSprinting; }

protected:
	virtual void BeginPlay() override;

private:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void StartSprint();
	void StopSprint();
	void Interact();
	void ToggleDiagnostics();

	UPROPERTY(VisibleAnywhere, Category = "UEGT1|Components")
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	UPROPERTY(VisibleAnywhere, Category = "UEGT1|Components")
	TObjectPtr<UUEGT1InteractionComponent> InteractionComponent;

	UPROPERTY(EditDefaultsOnly, Category = "UEGT1|Movement")
	float WalkSpeed = 480.0f;

	UPROPERTY(EditDefaultsOnly, Category = "UEGT1|Movement")
	float SprintSpeed = 760.0f;

	UPROPERTY(EditDefaultsOnly, Category = "UEGT1|Camera")
	float BaseFieldOfView = 92.0f;

	UPROPERTY(EditDefaultsOnly, Category = "UEGT1|Camera")
	float SprintFieldOfView = 98.0f;

	float StandingCameraHeight = 64.0f;
	float MovementTime = 0.0f;
	bool bSprinting = false;
};
