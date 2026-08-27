#include "Player/UEGT1ExplorerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Development/UEGT1DeveloperModeSubsystem.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Interaction/UEGT1InteractionComponent.h"
#include "UI/UEGT1HUD.h"

AUEGT1ExplorerCharacter::AUEGT1ExplorerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 92.0f);
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = false;
	Movement->MaxWalkSpeed = WalkSpeed;
	Movement->JumpZVelocity = 520.0f;
	Movement->AirControl = 0.35f;
	Movement->BrakingDecelerationWalking = 1800.0f;
	Movement->MaxStepHeight = 50.0f;

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.0f, 0.0f, StandingCameraHeight));
	FirstPersonCamera->bUsePawnControlRotation = true;
	FirstPersonCamera->SetFieldOfView(BaseFieldOfView);

	InteractionComponent = CreateDefaultSubobject<UUEGT1InteractionComponent>(TEXT("InteractionComponent"));
}

void AUEGT1ExplorerCharacter::BeginPlay()
{
	Super::BeginPlay();
	RefreshDeveloperMode();
}

void AUEGT1ExplorerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (IsDeveloperFlying())
	{
		const float VerticalInput = (bAscending ? 1.0f : 0.0f) - (bDescending ? 1.0f : 0.0f);
		if (!FMath::IsNearlyZero(VerticalInput))
		{
			AddMovementInput(FVector::UpVector, VerticalInput);
		}
	}

	const float HorizontalSpeed = GetVelocity().Size2D();
	const bool bMovingOnGround = HorizontalSpeed > 10.0f && GetCharacterMovement()->IsMovingOnGround();
	if (bMovingOnGround)
	{
		MovementTime += DeltaSeconds * (bSprinting ? 12.0f : 9.0f);
	}

	const float BobAmount = bMovingOnGround ? FMath::Sin(MovementTime) * (bSprinting ? 1.8f : 1.1f) : 0.0f;
	const FVector DesiredCameraLocation(0.0f, 0.0f, StandingCameraHeight + BobAmount);
	FirstPersonCamera->SetRelativeLocation(FMath::VInterpTo(FirstPersonCamera->GetRelativeLocation(), DesiredCameraLocation, DeltaSeconds, 12.0f));

	const float DesiredFov = bSprinting && HorizontalSpeed > 50.0f ? SprintFieldOfView : BaseFieldOfView;
	FirstPersonCamera->SetFieldOfView(FMath::FInterpTo(FirstPersonCamera->FieldOfView, DesiredFov, DeltaSeconds, 7.0f));
}

void AUEGT1ExplorerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AUEGT1ExplorerCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AUEGT1ExplorerCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AUEGT1ExplorerCharacter::Turn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AUEGT1ExplorerCharacter::LookUp);
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &AUEGT1ExplorerCharacter::StartJumpOrAscend);
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &AUEGT1ExplorerCharacter::StopJumpOrAscend);
	PlayerInputComponent->BindAction(TEXT("DeveloperDescend"), IE_Pressed, this, &AUEGT1ExplorerCharacter::StartDescend);
	PlayerInputComponent->BindAction(TEXT("DeveloperDescend"), IE_Released, this, &AUEGT1ExplorerCharacter::StopDescend);
	PlayerInputComponent->BindAction(TEXT("ToggleDeveloperMode"), IE_Pressed, this, &AUEGT1ExplorerCharacter::ToggleDeveloperMode);
	PlayerInputComponent->BindAction(TEXT("ToggleDeveloperFlight"), IE_Pressed, this, &AUEGT1ExplorerCharacter::ToggleDeveloperFlight);
	PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Pressed, this, &AUEGT1ExplorerCharacter::StartSprint);
	PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Released, this, &AUEGT1ExplorerCharacter::StopSprint);
	PlayerInputComponent->BindAction(TEXT("Interact"), IE_Pressed, this, &AUEGT1ExplorerCharacter::Interact);
	PlayerInputComponent->BindAction(TEXT("ToggleDiagnostics"), IE_Pressed, this, &AUEGT1ExplorerCharacter::ToggleDiagnostics);
}

void AUEGT1ExplorerCharacter::MoveForward(float Value)
{
	if (Controller && !FMath::IsNearlyZero(Value))
	{
		const FRotator MovementRotation = IsDeveloperFlying() ? Controller->GetControlRotation() : FRotator(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
		AddMovementInput(FRotationMatrix(MovementRotation).GetUnitAxis(EAxis::X), Value);
	}
}

void AUEGT1ExplorerCharacter::MoveRight(float Value)
{
	if (Controller && !FMath::IsNearlyZero(Value))
	{
		AddMovementInput(FRotationMatrix(FRotator(0.0f, Controller->GetControlRotation().Yaw, 0.0f)).GetUnitAxis(EAxis::Y), Value);
	}
}

void AUEGT1ExplorerCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void AUEGT1ExplorerCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void AUEGT1ExplorerCharacter::StartSprint()
{
	bSprinting = true;
	ApplyMovementTuning();
}

void AUEGT1ExplorerCharacter::StopSprint()
{
	bSprinting = false;
	ApplyMovementTuning();
}

void AUEGT1ExplorerCharacter::StartJumpOrAscend()
{
	if (IsDeveloperFlying())
	{
		bAscending = true;
		return;
	}
	Jump();
}

void AUEGT1ExplorerCharacter::StopJumpOrAscend()
{
	bAscending = false;
	StopJumping();
}

void AUEGT1ExplorerCharacter::StartDescend()
{
	bDescending = IsDeveloperFlying();
}

void AUEGT1ExplorerCharacter::StopDescend()
{
	bDescending = false;
}

void AUEGT1ExplorerCharacter::ToggleDeveloperMode()
{
	if (UUEGT1DeveloperModeSubsystem* DeveloperMode = GetGameInstance()->GetSubsystem<UUEGT1DeveloperModeSubsystem>())
	{
		DeveloperMode->SetEnabled(!DeveloperMode->IsEnabled());
		RefreshDeveloperMode();
	}
}

void AUEGT1ExplorerCharacter::ToggleDeveloperFlight()
{
	if (UUEGT1DeveloperModeSubsystem* DeveloperMode = GetGameInstance()->GetSubsystem<UUEGT1DeveloperModeSubsystem>())
	{
		DeveloperMode->SetFlightEnabled(!DeveloperMode->IsFlightEnabled());
		RefreshDeveloperMode();
	}
}

bool AUEGT1ExplorerCharacter::IsDeveloperModeEnabled() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	const UUEGT1DeveloperModeSubsystem* DeveloperMode = GameInstance ? GameInstance->GetSubsystem<UUEGT1DeveloperModeSubsystem>() : nullptr;
	return DeveloperMode && DeveloperMode->IsEnabled();
}

bool AUEGT1ExplorerCharacter::IsDeveloperFlying() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	const UUEGT1DeveloperModeSubsystem* DeveloperMode = GameInstance ? GameInstance->GetSubsystem<UUEGT1DeveloperModeSubsystem>() : nullptr;
	return DeveloperMode && DeveloperMode->IsFlightEnabled();
}

void AUEGT1ExplorerCharacter::RefreshDeveloperMode()
{
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (IsDeveloperFlying())
	{
		Movement->SetMovementMode(MOVE_Flying);
	}
	else if (Movement->MovementMode == MOVE_Flying)
	{
		Movement->SetMovementMode(MOVE_Falling);
	}
	bAscending = false;
	bDescending = false;
	ApplyMovementTuning();
}

void AUEGT1ExplorerCharacter::ApplyMovementTuning()
{
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	const bool bDeveloper = IsDeveloperModeEnabled();
	Movement->MaxWalkSpeed = bDeveloper ? (bSprinting ? DeveloperSprintSpeed : DeveloperWalkSpeed) : (bSprinting ? SprintSpeed : WalkSpeed);
	Movement->MaxFlySpeed = bDeveloper ? (bSprinting ? DeveloperSprintSpeed : DeveloperFlySpeed) : SprintSpeed;
	Movement->BrakingDecelerationFlying = bDeveloper ? 2800.0f : 0.0f;
	Movement->MaxAcceleration = bDeveloper ? 12000.0f : 2048.0f;
}

float AUEGT1ExplorerCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	return IsDeveloperModeEnabled() ? 0.0f : Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void AUEGT1ExplorerCharacter::Interact()
{
	InteractionComponent->TryInteract();
}

void AUEGT1ExplorerCharacter::ToggleDiagnostics()
{
	const APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (AUEGT1HUD* HUD = PlayerController ? Cast<AUEGT1HUD>(PlayerController->GetHUD()) : nullptr)
	{
		HUD->ToggleDiagnostics();
	}
}
