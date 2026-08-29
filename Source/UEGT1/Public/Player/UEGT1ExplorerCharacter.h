#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "UEGT1ExplorerCharacter.generated.h"

class UCameraComponent;
class UUEGT1InteractionComponent;

namespace UEGT1PlayerMovement
{
	constexpr float DefaultWalkSpeed = 480.0f;
}

UCLASS()
class UEGT1_API AUEGT1ExplorerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AUEGT1ExplorerCharacter();
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintPure, Category = "UEGT1|Interaction")
	UUEGT1InteractionComponent* GetInteractionComponent() const { return InteractionComponent; }

	UFUNCTION(BlueprintPure, Category = "UEGT1|Movement")
	bool IsSprinting() const { return bSprinting; }

	UFUNCTION(BlueprintPure, Category = "UEGT1|Developer Mode")
	bool IsDeveloperModeEnabled() const;

	UFUNCTION(BlueprintPure, Category = "UEGT1|Developer Mode")
	bool IsDeveloperFlying() const;

	void RefreshDeveloperMode();

protected:
	virtual void BeginPlay() override;

private:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void StartSprint();
	void StopSprint();
	void StartJumpOrAscend();
	void StopJumpOrAscend();
	void StartDescend();
	void StopDescend();
	void ToggleDeveloperMode();
	void ToggleDeveloperFlight();
	void ApplyMovementTuning();
	void Interact();
	void ToggleDiagnostics();
	void CycleSimulationInspector();
	void SaveTownSimulation();
	void LoadTownSimulation();

	UPROPERTY(VisibleAnywhere, Category = "UEGT1|Components")
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	UPROPERTY(VisibleAnywhere, Category = "UEGT1|Components")
	TObjectPtr<UUEGT1InteractionComponent> InteractionComponent;

	UPROPERTY(EditDefaultsOnly, Category = "UEGT1|Movement")
	float WalkSpeed = UEGT1PlayerMovement::DefaultWalkSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "UEGT1|Movement")
	float SprintSpeed = 760.0f;

	UPROPERTY(EditDefaultsOnly, Category = "UEGT1|Developer Mode")
	float DeveloperWalkSpeed = 2100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "UEGT1|Developer Mode")
	float DeveloperSprintSpeed = 4200.0f;

	UPROPERTY(EditDefaultsOnly, Category = "UEGT1|Developer Mode")
	float DeveloperFlySpeed = 3200.0f;

	UPROPERTY(EditDefaultsOnly, Category = "UEGT1|Camera")
	float BaseFieldOfView = 92.0f;

	UPROPERTY(EditDefaultsOnly, Category = "UEGT1|Camera")
	float SprintFieldOfView = 98.0f;

	float StandingCameraHeight = 64.0f;
	float MovementTime = 0.0f;
	bool bSprinting = false;
	bool bAscending = false;
	bool bDescending = false;
};
