#include "Player/UEGT1ExplorerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
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
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void AUEGT1ExplorerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

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
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Pressed, this, &AUEGT1ExplorerCharacter::StartSprint);
	PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Released, this, &AUEGT1ExplorerCharacter::StopSprint);
	PlayerInputComponent->BindAction(TEXT("Interact"), IE_Pressed, this, &AUEGT1ExplorerCharacter::Interact);
	PlayerInputComponent->BindAction(TEXT("ToggleDiagnostics"), IE_Pressed, this, &AUEGT1ExplorerCharacter::ToggleDiagnostics);
}

void AUEGT1ExplorerCharacter::MoveForward(float Value)
{
	if (Controller && !FMath::IsNearlyZero(Value))
	{
		AddMovementInput(FRotationMatrix(FRotator(0.0f, Controller->GetControlRotation().Yaw, 0.0f)).GetUnitAxis(EAxis::X), Value);
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
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void AUEGT1ExplorerCharacter::StopSprint()
{
	bSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
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
