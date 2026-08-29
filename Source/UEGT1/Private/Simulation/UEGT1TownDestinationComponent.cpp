#include "Simulation/UEGT1TownDestinationComponent.h"

#include "Simulation/UEGT1TownSimulationSubsystem.h"
#include "World/UEGT1TownGeneration.h"

UUEGT1TownDestinationComponent::UUEGT1TownDestinationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetMobility(EComponentMobility::Static);
}

void UUEGT1TownDestinationComponent::ConfigureFromGeneratedLot(const FUEGT1GeneratedTownLot& Lot)
{
	VenueId = Lot.LotId;
	DisplayName = Lot.DisplayName;
	JobTitle = Lot.JobTitle;
	BusinessType = Lot.BusinessType;
	VenueType = Lot.VenueType;
	Capacity = Lot.Capacity;
	HourlyRate = Lot.HourlyRate;
	OpeningHour = Lot.OpeningHour;
	ClosingHour = Lot.ClosingHour;
	AccessPoint = Lot.AccessPoint;
	EntranceLocation = Lot.EntranceLocation;
	InteriorEntryLocation = Lot.InteriorEntryLocation;
	KitchenLocation = Lot.KitchenLocation;
	BathroomLocation = Lot.BathroomLocation;
	ShowerLocation = Lot.ShowerLocation;
	ActivityLocation = Lot.ActivityLocation;
	AmenityBedLocations = Lot.AmenityBedLocations;
	Beds = Lot.Beds;
	SetRelativeLocation(Lot.Center);
}

FUEGT1TownVenueState UUEGT1TownDestinationComponent::MakeVenueState() const
{
	FUEGT1TownVenueState State;
	State.VenueId = VenueId;
	State.DisplayName = DisplayName;
	State.JobTitle = JobTitle;
	State.BusinessType = BusinessType;
	State.VenueType = VenueType;
	State.WorldLocation = GetComponentLocation();
	State.AccessPoint = GetOwner() ? GetOwner()->GetActorTransform().TransformPosition(AccessPoint) : AccessPoint;
	const FTransform OwnerTransform = GetOwner() ? GetOwner()->GetActorTransform() : FTransform::Identity;
	State.EntranceLocation = OwnerTransform.TransformPosition(EntranceLocation);
	State.InteriorEntryLocation = OwnerTransform.TransformPosition(InteriorEntryLocation);
	State.KitchenLocation = OwnerTransform.TransformPosition(KitchenLocation);
	State.BathroomLocation = OwnerTransform.TransformPosition(BathroomLocation);
	State.ShowerLocation = OwnerTransform.TransformPosition(ShowerLocation);
	State.ActivityLocation = OwnerTransform.TransformPosition(ActivityLocation);
	for (const FVector& AmenityBedLocation : AmenityBedLocations)
	{
		State.AmenityBedLocations.Add(OwnerTransform.TransformPosition(AmenityBedLocation));
	}
	State.OpeningHour = OpeningHour;
	State.ClosingHour = ClosingHour;
	State.Capacity = Capacity;
	State.HourlyRate = HourlyRate;
	State.bReachable = bReachable;
	for (const FUEGT1TownBedState& Bed : Beds)
	{
		FUEGT1TownBedState WorldBed = Bed;
		WorldBed.WorldLocation = OwnerTransform.TransformPosition(Bed.WorldLocation);
		WorldBed.Rotation = OwnerTransform.TransformRotation(Bed.Rotation.Quaternion()).Rotator();
		State.Beds.Add(MoveTemp(WorldBed));
	}
	return State;
}

void UUEGT1TownDestinationComponent::BeginPlay()
{
	Super::BeginPlay();
	if (UUEGT1TownSimulationSubsystem* Simulation = GetWorld()->GetSubsystem<UUEGT1TownSimulationSubsystem>())
	{
		Simulation->RegisterDestination(this);
	}
}

void UUEGT1TownDestinationComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (UUEGT1TownSimulationSubsystem* Simulation = World->GetSubsystem<UUEGT1TownSimulationSubsystem>())
		{
			Simulation->UnregisterDestination(this);
		}
	}
	Super::EndPlay(EndPlayReason);
}
