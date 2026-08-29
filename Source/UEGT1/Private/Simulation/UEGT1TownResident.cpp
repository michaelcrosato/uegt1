#include "Simulation/UEGT1TownResident.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "World/UEGT1VisualMaterials.h"

void FUEGT1ResidentVisualMotion::Snap(const FVector& Position)
{
	StartPosition = Position;
	TargetPosition = Position;
	CurrentPosition = Position;
	ElapsedSeconds = 0.0f;
	DurationSeconds = 0.0f;
}

void FUEGT1ResidentVisualMotion::Retarget(const FVector& InCurrentPosition, const FVector& NewTarget, float InDurationSeconds)
{
	StartPosition = InCurrentPosition;
	CurrentPosition = InCurrentPosition;
	TargetPosition = NewTarget;
	ElapsedSeconds = 0.0f;
	DurationSeconds = FMath::Max(InDurationSeconds, 0.0f);
	if (DurationSeconds <= SMALL_NUMBER || StartPosition.Equals(TargetPosition, 0.1f))
	{
		Snap(TargetPosition);
	}
}

FVector FUEGT1ResidentVisualMotion::Advance(float DeltaSeconds)
{
	if (!IsMoving())
	{
		return CurrentPosition;
	}
	ElapsedSeconds = FMath::Min(ElapsedSeconds + FMath::Max(DeltaSeconds, 0.0f), DurationSeconds);
	const float Alpha = DurationSeconds > SMALL_NUMBER ? ElapsedSeconds / DurationSeconds : 1.0f;
	CurrentPosition = FMath::Lerp(StartPosition, TargetPosition, Alpha);
	if (ElapsedSeconds >= DurationSeconds)
	{
		CurrentPosition = TargetPosition;
	}
	return CurrentPosition;
}

bool FUEGT1ResidentVisualMotion::IsMoving() const
{
	return ElapsedSeconds < DurationSeconds && !CurrentPosition.Equals(TargetPosition, 0.1f);
}

AUEGT1TownResident::AUEGT1TownResident()
{
	PrimaryActorTick.bCanEverTick = false;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	ShapeMaterial = MaterialFinder.Object;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	BodyMesh->SetupAttachment(SceneRoot);
	BodyMesh->SetStaticMesh(CylinderFinder.Object);
	BodyMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 58.0f));
	BodyMesh->SetRelativeScale3D(FVector(0.30f, 0.30f, 0.85f));
	HeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Head"));
	HeadMesh->SetupAttachment(SceneRoot);
	HeadMesh->SetStaticMesh(SphereFinder.Object);
	HeadMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 142.0f));
	HeadMesh->SetRelativeScale3D(FVector(0.34f));
	for (UStaticMeshComponent* Component : { BodyMesh, HeadMesh })
	{
		Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Component->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
		Component->SetCanEverAffectNavigation(false);
		Component->CastShadow = true;
	}
}

void AUEGT1TownResident::InitializeResident(FName InNpcId, int32 VisualSeed)
{
	NpcId = InNpcId;
	Tags.AddUnique(TEXT("UEGT1SimulatedResident"));
	FRandomStream Random(VisualSeed + GetTypeHash(NpcId));
	const FLinearColor Clothing = FLinearColor::MakeFromHSV8(Random.RandRange(0, 255), 150, 205);
	BodyMaterial = UEGT1VisualMaterials::Apply(this, BodyMesh, ShapeMaterial, EUEGT1VisualMaterial::Surface,
		Clothing, 0.72f, 0.25f);
	HeadMaterial = UEGT1VisualMaterials::Apply(this, HeadMesh, ShapeMaterial, EUEGT1VisualMaterial::Surface,
		FLinearColor(0.72f, 0.48f, 0.32f), 0.82f, 0.18f);
}

void AUEGT1TownResident::ApplySimulationPosition(const FVector& Position)
{
	VisualMotion.Snap(Position);
	SetActorLocation(Position);
}

void AUEGT1TownResident::SetSimulationTarget(const FVector& Position, float BlendDurationSeconds)
{
	const FVector CurrentPosition = GetActorLocation();
	const FVector Direction = Position - CurrentPosition;
	if (Direction.SizeSquared2D() > 4.0f)
	{
		TargetRotation = Direction.Rotation();
	}
	VisualMotion.Retarget(CurrentPosition, Position, BlendDurationSeconds);
}

void AUEGT1TownResident::AdvanceVisualMovement(float DeltaSeconds)
{
	if (!VisualMotion.IsMoving())
	{
		return;
	}
	SetActorLocation(VisualMotion.Advance(DeltaSeconds));
	SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaSeconds, 12.0f));
}
