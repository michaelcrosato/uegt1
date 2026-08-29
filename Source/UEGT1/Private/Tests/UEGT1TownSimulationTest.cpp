#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "Kismet/GameplayStatics.h"
#include "Player/UEGT1ExplorerCharacter.h"
#include "Simulation/UEGT1TownSimulationModel.h"
#include "Simulation/UEGT1TownResident.h"
#include "Simulation/UEGT1TownSimulationSaveGame.h"
#include "Simulation/UEGT1TownSimulationSettings.h"
#include "World/UEGT1TownGeneration.h"
#include "World/UEGT1WorldLayout.h"

namespace
{
	TArray<FUEGT1TownVenueState> MakeVenueStates(const FUEGT1GeneratedTownLayout& Layout)
	{
		TArray<FUEGT1TownVenueState> Venues;
		for (const FUEGT1GeneratedTownLot& Lot : Layout.Lots)
		{
			FUEGT1TownVenueState Venue;
			Venue.VenueId = Lot.LotId;
			Venue.DisplayName = Lot.DisplayName;
			Venue.JobTitle = Lot.JobTitle;
			Venue.BusinessType = Lot.BusinessType;
			Venue.VenueType = Lot.VenueType;
			Venue.WorldLocation = Lot.Center;
			Venue.AccessPoint = Lot.AccessPoint;
			Venue.EntranceLocation = Lot.EntranceLocation;
			Venue.InteriorEntryLocation = Lot.InteriorEntryLocation;
			Venue.KitchenLocation = Lot.KitchenLocation;
			Venue.BathroomLocation = Lot.BathroomLocation;
			Venue.ShowerLocation = Lot.ShowerLocation;
			Venue.ActivityLocation = Lot.ActivityLocation;
			Venue.AmenityBedLocations = Lot.AmenityBedLocations;
			Venue.Capacity = Lot.Capacity;
			Venue.HourlyRate = Lot.HourlyRate;
			Venue.OpeningHour = Lot.OpeningHour;
			Venue.ClosingHour = Lot.ClosingHour;
			Venue.Beds = Lot.Beds;
			Venues.Add(Venue);
		}
		return Venues;
	}

	FUEGT1TownSimulationModel MakeModel(int32 Seed = 7319, int32 NPCCount = 100)
	{
		const FUEGT1SimulationTuning Tuning = UUEGT1TownSimulationSettings::Get().MakeTuning();
		const FUEGT1GeneratedTownLayout Layout = UEGT1TownGeneration::Generate(Seed, Tuning);
		FUEGT1TownSimulationModel Model;
		Model.Initialize(Seed, NPCCount, MakeVenueStates(Layout), Tuning);
		return Model;
	}

	const FUEGT1ActionDefinition* FindDefinition(const FUEGT1SimulationTuning& Tuning, EUEGT1SimActionType Action,
		EUEGT1TownVenueType Venue)
	{
		return Tuning.ActionDefinitions.FindByPredicate([Action, Venue](const FUEGT1ActionDefinition& Definition)
		{
			return Definition.Action == Action && Definition.VenueType == Venue;
		});
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUEGT1ResidentVisualInterpolationTest,
	"UEGT1.TownSimulation.ResidentVisualInterpolation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUEGT1ResidentVisualInterpolationTest::RunTest(const FString& Parameters)
{
	FUEGT1ResidentVisualMotion Motion;
	const FVector Start(100.0f, 200.0f, 92.0f);
	const FVector Target(500.0f, 200.0f, 92.0f);
	Motion.Snap(Start);
	Motion.Retarget(Start, Target, 1.0f);
	TestEqual(TEXT("Retargeting does not teleport the rendered resident"), Motion.GetCurrentPosition(), Start);
	TestTrue(TEXT("A non-zero visual leg begins interpolating"), Motion.IsMoving());
	TestEqual(TEXT("The resident renders one quarter of the way through a one-second leg"),
		Motion.Advance(0.25f), FVector(200.0f, 200.0f, 92.0f));
	TestEqual(TEXT("The resident renders continuously at the halfway sample"),
		Motion.Advance(0.25f), FVector(300.0f, 200.0f, 92.0f));
	TestEqual(TEXT("The resident finishes exactly at the authoritative simulation target"),
		Motion.Advance(0.50f), Target);
	TestFalse(TEXT("The completed visual leg no longer reports movement"), Motion.IsMoving());
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUEGT1CitizenMovementPaceTest,
	"UEGT1.TownSimulation.CitizenMovementPace",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUEGT1CitizenMovementPaceTest::RunTest(const FString& Parameters)
{
	const FUEGT1SimulationTuning Tuning = UUEGT1TownSimulationSettings::Get().MakeTuning();
	TestTrue(TEXT("Citizens walk slower than the normal player"),
		Tuning.CitizenWalkSpeedCentimetersPerSecond < UEGT1PlayerMovement::DefaultWalkSpeed);
	TestTrue(TEXT("Citizen pace remains only slightly slower than the player"),
		Tuning.CitizenWalkSpeedCentimetersPerSecond >= UEGT1PlayerMovement::DefaultWalkSpeed * 0.85f);
	TestEqual(TEXT("The configured citizen presentation pace is 440 cm/s"),
		Tuning.CitizenWalkSpeedCentimetersPerSecond, 440.0f);
	TestEqual(TEXT("Simulation travel converts back to the configured real-time citizen pace"),
		Tuning.GetTravelSpeedCentimetersPerSimulationMinute() * Tuning.SimMinutesPerRealSecond,
		Tuning.CitizenWalkSpeedCentimetersPerSecond);
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUEGT1DeterministicTownGenerationTest,
	"UEGT1.TownSimulation.DeterministicGeneration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUEGT1DeterministicTownGenerationTest::RunTest(const FString& Parameters)
{
	const FUEGT1SimulationTuning Tuning = UUEGT1TownSimulationSettings::Get().MakeTuning();
	const FUEGT1GeneratedTownLayout First = UEGT1TownGeneration::Generate(7319, Tuning);
	const FUEGT1GeneratedTownLayout Again = UEGT1TownGeneration::Generate(7319, Tuning);
	const FUEGT1GeneratedTownLayout Different = UEGT1TownGeneration::Generate(7320, Tuning);
	TestTrue(TEXT("Generated streets and venue access points form one connected query graph"), UEGT1TownGeneration::IsFullyConnected(First));
	TestEqual(TEXT("The job-expanded town has eighteen connected streets"), First.Streets.Num(), 18);
	TestEqual(TEXT("The expanded street grid has fifty-six intersections"), First.Intersections.Num(), 56);
	TestEqual(TEXT("The large town has fifty-two queryable lots"), First.Lots.Num(), 52);
	TestTrue(TEXT("The town expands primarily west while retaining the sanctuary-facing east edge"),
		First.BoundsMin.X <= -22000.0f && First.BoundsMax.X <= 5000.0f &&
		FMath::Abs(First.BoundsMin.X) > First.BoundsMax.X * 3.0f);

	TMap<EUEGT1TownVenueType, int32> Counts;
	TSet<FName> BedIds;
	int32 BedCount = 0;
	bool bEveryHomeHasPhysicalBeds = true;
	bool bEveryBuildingHasAmenities = true;
	bool bLotsAvoidPrimaryRoutes = true;
	bool bEveryLotFitsWorldMap = true;
	bool bSameSeedMatches = First.Lots.Num() == Again.Lots.Num();
	bool bDifferentSeedVaries = false;
	for (int32 Index = 0; Index < First.Lots.Num(); ++Index)
	{
		const FUEGT1GeneratedTownLot& Lot = First.Lots[Index];
		++Counts.FindOrAdd(Lot.VenueType);
		bEveryHomeHasPhysicalBeds &= Lot.VenueType != EUEGT1TownVenueType::Home || Lot.Beds.Num() == Lot.Capacity;
		if (Lot.VenueType != EUEGT1TownVenueType::Park)
		{
			const FVector Forward = Lot.FacingRotation.Vector();
			const FVector Right = FRotationMatrix(Lot.FacingRotation).GetScaledAxis(EAxis::Y);
			auto IsInside = [&](const FVector& Point)
			{
				const FVector Delta = Point - Lot.Center;
				return FMath::Abs(FVector::DotProduct(Delta, Forward)) < Lot.Footprint.X * 0.5f &&
					FMath::Abs(FVector::DotProduct(Delta, Right)) < Lot.Footprint.Y * 0.5f;
			};
			bEveryBuildingHasAmenities &= !Lot.EntranceLocation.IsNearlyZero() && IsInside(Lot.InteriorEntryLocation) &&
				IsInside(Lot.KitchenLocation) && IsInside(Lot.BathroomLocation) && IsInside(Lot.ShowerLocation) &&
				((Lot.VenueType == EUEGT1TownVenueType::Home && !Lot.Beds.IsEmpty()) ||
				 (Lot.VenueType != EUEGT1TownVenueType::Home && !Lot.AmenityBedLocations.IsEmpty()));
		}
		bLotsAvoidPrimaryRoutes &= !UEGT1WorldLayout::IsPrimaryRoute(Lot.Center,
			FMath::Max(Lot.Footprint.X, Lot.Footprint.Y) * 0.5f);
		bEveryLotFitsWorldMap &= Lot.Center.X >= UEGT1WorldLayout::GetWorldMinX() &&
			Lot.Center.X <= UEGT1WorldLayout::GetWorldMaxX() &&
			Lot.Center.Y >= UEGT1WorldLayout::GetWorldMinY() &&
			Lot.Center.Y <= UEGT1WorldLayout::GetWorldMaxY();
		for (const FUEGT1TownBedState& Bed : Lot.Beds)
		{
			++BedCount;
			BedIds.Add(Bed.BedId);
		}
		bSameSeedMatches &= Lot.LotId == Again.Lots[Index].LotId && Lot.VenueType == Again.Lots[Index].VenueType &&
			Lot.Center.Equals(Again.Lots[Index].Center) && Lot.Footprint.Equals(Again.Lots[Index].Footprint, KINDA_SMALL_NUMBER) &&
			Lot.Beds.Num() == Again.Lots[Index].Beds.Num();
		bDifferentSeedVaries |= Lot.VenueType != Different.Lots[Index].VenueType ||
			!Lot.Footprint.Equals(Different.Lots[Index].Footprint, KINDA_SMALL_NUMBER);
	}
	TestTrue(TEXT("The same seed reproduces lot identity, use, placement, and dimensions"), bSameSeedMatches);
	TestTrue(TEXT("A different seed changes the generated town composition"), bDifferentSeedVaries);
	TestEqual(TEXT("Twenty-five homes house one hundred residents"), Counts.FindRef(EUEGT1TownVenueType::Home), 25);
	TestEqual(TEXT("The large town includes two food venues"), Counts.FindRef(EUEGT1TownVenueType::FoodVenue), 2);
	TestEqual(TEXT("The large town includes twenty distinct job sites"), Counts.FindRef(EUEGT1TownVenueType::Workplace), 20);
	TestEqual(TEXT("The large town includes two social venues"), Counts.FindRef(EUEGT1TownVenueType::SocialVenue), 2);
	TestEqual(TEXT("The large town includes three free social parks"), Counts.FindRef(EUEGT1TownVenueType::Park), 3);
	TestTrue(TEXT("Every home contains one physical bed slot per resident capacity"), bEveryHomeHasPhysicalBeds);
	TestTrue(TEXT("Every building has an enterable interior with a kitchen, bathroom, shower, and bed"), bEveryBuildingHasAmenities);
	TestEqual(TEXT("The generated town contains exactly one hundred physical beds"), BedCount, 100);
	TestEqual(TEXT("Every generated bed has a unique identity"), BedIds.Num(), BedCount);
	TestTrue(TEXT("Town lots preserve all primary Waystone routes"), bLotsAvoidPrimaryRoutes);
	TestTrue(TEXT("Every generated venue fits inside the full-island map bounds"), bEveryLotFitsWorldMap);
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUEGT1DoorAwareRoutingTest,
	"UEGT1.TownSimulation.DoorAwareRouting",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUEGT1DoorAwareRoutingTest::RunTest(const FString& Parameters)
{
	const FUEGT1SimulationTuning Tuning = UUEGT1TownSimulationSettings::Get().MakeTuning();
	const FUEGT1GeneratedTownLayout Layout = UEGT1TownGeneration::Generate(7319, Tuning);
	const TArray<FUEGT1TownVenueState> Venues = MakeVenueStates(Layout);
	const FUEGT1TownVenueState* Source = Venues.FindByPredicate([](const FUEGT1TownVenueState& Venue)
	{
		return Venue.VenueType == EUEGT1TownVenueType::Home;
	});
	const FUEGT1TownVenueState* Destination = Venues.FindByPredicate([Source](const FUEGT1TownVenueState& Venue)
	{
		return Source && Venue.VenueId != Source->VenueId && Venue.VenueType == EUEGT1TownVenueType::Workplace;
	});
	TestNotNull(TEXT("A source building exists"), Source);
	TestNotNull(TEXT("A destination building exists"), Destination);
	if (!Source || !Destination)
	{
		return false;
	}
	const TArray<FVector> Path = UEGT1TownGeneration::BuildVenuePath(Source->ActivityLocation, Source,
		*Destination, Destination->ActivityLocation);
	auto FindPoint = [&Path](const FVector& Point)
	{
		return Path.IndexOfByPredicate([&Point](const FVector& Candidate) { return Candidate.Equals(Point, 1.0f); });
	};
	const int32 SourceInterior = FindPoint(Source->InteriorEntryLocation);
	const int32 SourceDoor = FindPoint(Source->EntranceLocation);
	const int32 SourceStreet = FindPoint(Source->AccessPoint);
	const int32 DestinationStreet = FindPoint(Destination->AccessPoint);
	const int32 DestinationDoor = FindPoint(Destination->EntranceLocation);
	const int32 DestinationInterior = FindPoint(Destination->InteriorEntryLocation);
	TestTrue(TEXT("The route exits through the source interior and door in order"),
		SourceInterior > 0 && SourceDoor > SourceInterior && SourceStreet > SourceDoor);
	TestTrue(TEXT("The route enters through the destination street access and door in order"),
		DestinationStreet > SourceStreet && DestinationDoor > DestinationStreet && DestinationInterior > DestinationDoor);
	TestTrue(TEXT("The route ends at the requested activity station"), Path.Last().Equals(Destination->ActivityLocation, 1.0f));

	auto SegmentCrossesBuilding = [](const FVector& SegmentStart, const FVector& SegmentEnd,
		const FUEGT1GeneratedTownLot& Lot)
	{
		const FVector Forward = Lot.FacingRotation.Vector();
		const FVector Right = FRotationMatrix(Lot.FacingRotation).GetScaledAxis(EAxis::Y);
		const FVector StartDelta = SegmentStart - Lot.Center;
		const FVector EndDelta = SegmentEnd - Lot.Center;
		const FVector2D A(FVector::DotProduct(StartDelta, Forward), FVector::DotProduct(StartDelta, Right));
		const FVector2D B(FVector::DotProduct(EndDelta, Forward), FVector::DotProduct(EndDelta, Right));
		const FVector2D Delta = B - A;
		const FVector2D Half(Lot.Footprint.X * 0.5f - 28.0f, Lot.Footprint.Y * 0.5f - 28.0f);
		float MinimumT = 0.0f;
		float MaximumT = 1.0f;
		auto ClipAxis = [&](float Coordinate, float Direction, float Extent)
		{
			if (FMath::Abs(Direction) < KINDA_SMALL_NUMBER)
			{
				return FMath::Abs(Coordinate) <= Extent;
			}
			float First = (-Extent - Coordinate) / Direction;
			float Last = (Extent - Coordinate) / Direction;
			if (First > Last)
			{
				Swap(First, Last);
			}
			MinimumT = FMath::Max(MinimumT, First);
			MaximumT = FMath::Min(MaximumT, Last);
			return MinimumT <= MaximumT;
		};
		return ClipAxis(A.X, Delta.X, Half.X) && ClipAxis(A.Y, Delta.Y, Half.Y);
	};
	bool bAllRoutesAvoidUnrelatedBuildings = true;
	for (const FUEGT1TownVenueState& From : Venues)
	{
		for (const FUEGT1TownVenueState& To : Venues)
		{
			if (From.VenueId == To.VenueId)
			{
				continue;
			}
			const TArray<FVector> CandidatePath = UEGT1TownGeneration::BuildVenuePath(
				From.ActivityLocation, &From, To, To.ActivityLocation);
			for (int32 SegmentIndex = 1; SegmentIndex < CandidatePath.Num(); ++SegmentIndex)
			{
				for (const FUEGT1GeneratedTownLot& Building : Layout.Lots)
				{
					if (Building.VenueType != EUEGT1TownVenueType::Park && Building.LotId != From.VenueId &&
						Building.LotId != To.VenueId && SegmentCrossesBuilding(CandidatePath[SegmentIndex - 1],
							CandidatePath[SegmentIndex], Building))
					{
						bAllRoutesAvoidUnrelatedBuildings = false;
						break;
					}
				}
			}
		}
	}
	TestTrue(TEXT("All generated inter-venue paths avoid every unrelated building footprint"),
		bAllRoutesAvoidUnrelatedBuildings);
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUEGT1HundredResidentHousingTest,
	"UEGT1.TownSimulation.HundredResidentHousing",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUEGT1HundredResidentHousingTest::RunTest(const FString& Parameters)
{
	FUEGT1TownSimulationModel Model = MakeModel(7319, 100);
	TestTrue(TEXT("The one-hundred-resident model initializes"), Model.IsInitialized());
	TestEqual(TEXT("The town spawns exactly one hundred modeled citizens"), Model.GetNPCs().Num(), 100);
	TestEqual(TEXT("The town exposes exactly one hundred generated beds"), Model.GetBedCount(), 100);
	TestTrue(TEXT("Every citizen owns a unique bed belonging to their assigned home"), Model.HasCompleteBedAssignments());
	TSet<FName> AssignedBeds;
	for (const FUEGT1NPCSimulationState& NPC : Model.GetNPCs())
	{
		AssignedBeds.Add(NPC.BedId);
		TestEqual(TEXT("Every four-person home gives each resident three explicit relationships"),
			NPC.HouseholdRelationships.Num(), 3);
	}
	TestEqual(TEXT("No two citizens share a bed"), AssignedBeds.Num(), Model.GetNPCs().Num());
	TestTrue(TEXT("Every pair sharing a home has a clear, symmetric relationship"), Model.HasCompleteHouseholdRelationships());
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUEGT1JobCatalogAndHoursTest,
	"UEGT1.TownSimulation.JobCatalogAndHours",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUEGT1JobCatalogAndHoursTest::RunTest(const FString& Parameters)
{
	const FUEGT1SimulationTuning Tuning = UUEGT1TownSimulationSettings::Get().MakeTuning();
	TestEqual(TEXT("The town exposes twenty named job definitions"), Tuning.JobDefinitions.Num(), 20);
	TSet<FName> JobIds;
	int32 NightJobs = 0;
	int32 AlwaysOpenJobs = 0;
	int32 LimitedJobs = 0;
	int32 DaytimeJobs = 0;
	for (const FUEGT1JobDefinition& Job : Tuning.JobDefinitions)
	{
		JobIds.Add(Job.JobId);
		TestFalse(TEXT("Every job has a player-readable name"), Job.DisplayName.IsEmpty());
		TestFalse(TEXT("Every job operates from a named real business"), Job.BusinessName.IsEmpty());
		TestFalse(TEXT("Every job declares its business type"), Job.BusinessType.IsEmpty());
		TestTrue(TEXT("Every job pays a positive hourly rate"), Job.HourlyRate > 0.0f);
		AlwaysOpenJobs += FMath::IsNearlyEqual(Job.OpeningHour, Job.ClosingHour) ? 1 : 0;
		NightJobs += Job.OpeningHour > Job.ClosingHour ? 1 : 0;
		LimitedJobs += FMath::IsNearlyEqual(Job.OpeningHour, 10.0f) && FMath::IsNearlyEqual(Job.ClosingHour, 16.0f) ? 1 : 0;
		DaytimeJobs += Job.OpeningHour >= 6.0f && Job.ClosingHour <= 22.0f && Job.OpeningHour < Job.ClosingHour ? 1 : 0;
	}
	TestEqual(TEXT("All twenty job identities are unique"), JobIds.Num(), 20);
	TestEqual(TEXT("Two jobs operate overnight"), NightJobs, 2);
	TestEqual(TEXT("Two jobs can be done around the clock"), AlwaysOpenJobs, 2);
	TestEqual(TEXT("Four limited-hour jobs run from 10 AM to 4 PM"), LimitedJobs, 4);
	TestTrue(TEXT("Most jobs operate between 6 AM and 10 PM"), DaytimeJobs > Tuning.JobDefinitions.Num() / 2);

	const FUEGT1GeneratedTownLayout Layout = UEGT1TownGeneration::Generate(7319, Tuning);
	int32 GeneratedJobs = 0;
	for (const FUEGT1GeneratedTownLot& Lot : Layout.Lots)
	{
		if (Lot.VenueType == EUEGT1TownVenueType::Workplace)
		{
			++GeneratedJobs;
			TestFalse(TEXT("Each generated job site keeps its business name"), Lot.DisplayName.IsEmpty());
			TestFalse(TEXT("Each generated job site keeps its role title"), Lot.JobTitle.IsEmpty());
			TestFalse(TEXT("Each generated job site keeps its business type"), Lot.BusinessType.IsEmpty());
			TestTrue(TEXT("Each generated job site keeps its hourly rate"), Lot.HourlyRate > 0.0f);
		}
	}
	TestEqual(TEXT("All job definitions have a physical destination around town"), GeneratedJobs, 20);

	FUEGT1TownSimulationModel NightModel = MakeModel(7319, 1);
	FUEGT1TownSimulationSnapshot NightSnapshot = NightModel.MakeSnapshot();
	NightSnapshot.AbsoluteMinutes = 23.0f * 60.0f;
	TestTrue(TEXT("A night-time snapshot restores for operational checks"), NightModel.Restore(NightSnapshot, Tuning));
	FUEGT1NPCSimulationState* NightWorker = NightModel.FindNPC(TEXT("Resident_001"));
	for (const FName NightJobId : { FName(TEXT("NightWatch")), FName(TEXT("NightNurse")) })
	{
		FUEGT1TownVenueState* NightJob = NightModel.FindVenue(NightJobId);
		TestNotNull(TEXT("Each overnight role has a generated destination"), NightJob);
		if (NightWorker && NightJob)
		{
			NightWorker->JobId = NightJobId;
			NightWorker->CurrentVenueId = NightJobId;
			NightWorker->WorldLocation = NightJob->WorldLocation + FVector(0.0f, 0.0f, 92.0f);
			NightWorker->Money = 0.0f;
			NightWorker->Needs = FUEGT1NeedState{ 1.0f, 1.0f, 1.0f, 1.0f };
			const FUEGT1ActionUtilityScore NightChoice = NightModel.EvaluateBestAction(0, true);
			TestEqual(TEXT("The assigned overnight role can be selected at 11 PM"), NightChoice.Action, EUEGT1SimActionType::Work);
			TestEqual(TEXT("The worker selects their assigned overnight destination"), NightChoice.DestinationId, NightJobId);
			TestTrue(TEXT("The overnight site remains open for the complete paid hour"), NightJob->IsOpenForDuration(23.0f, 60.0f));
		}
	}
	for (const FName AlwaysOpenJobId : { FName(TEXT("EmergencyDispatcher")), FName(TEXT("TransitOperator")) })
	{
		const FUEGT1TownVenueState* AlwaysOpenJob = NightModel.GetVenues().FindByPredicate([AlwaysOpenJobId](const FUEGT1TownVenueState& Venue)
		{
			return Venue.VenueId == AlwaysOpenJobId;
		});
		TestTrue(TEXT("Each 24-hour job accepts an hour at any representative time"), AlwaysOpenJob &&
			AlwaysOpenJob->IsOpenForDuration(0.0f, 60.0f) && AlwaysOpenJob->IsOpenForDuration(12.0f, 60.0f) &&
			AlwaysOpenJob->IsOpenForDuration(23.0f, 60.0f));
	}
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUEGT1PlayerActivityParityTest,
	"UEGT1.TownSimulation.PlayerActivityParity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUEGT1PlayerActivityParityTest::RunTest(const FString& Parameters)
{
	const FUEGT1SimulationTuning Tuning = UUEGT1TownSimulationSettings::Get().MakeTuning();
	for (const FUEGT1ActionDefinition& Definition : Tuning.ActionDefinitions)
	{
		FUEGT1TownSimulationModel Model = MakeModel(7319, 1);
		FUEGT1TownSimulationSnapshot Snapshot = Model.MakeSnapshot();
		Snapshot.AbsoluteMinutes = Definition.VenueType == EUEGT1TownVenueType::SocialVenue ? 10.0f * 60.0f : 6.0f * 60.0f;
		TestTrue(TEXT("The activity test clock restores"), Model.Restore(Snapshot, Tuning));
		const FUEGT1TownVenueState* Venue = Model.GetVenues().FindByPredicate([&Definition](const FUEGT1TownVenueState& Candidate)
		{
			if (Definition.Action == EUEGT1SimActionType::Work)
			{
				return Candidate.VenueId == FName(TEXT("EmergencyDispatcher"));
			}
			return Candidate.VenueType == Definition.VenueType;
		});
		TestNotNull(TEXT("Every NPC activity has a physical player venue"), Venue);
		if (!Venue)
		{
			continue;
		}
		FString Reason;
		TestTrue(TEXT("The player can start each activity NPCs can perform"),
			Model.CanPlayerPerformActivity(Definition.Action, Venue->VenueId, Reason));
		const float MoneyBefore = Model.GetPlayer().Money;
		const float NeedBefore = Definition.Action == EUEGT1SimActionType::Sleep ? Model.GetPlayer().Needs.Energy :
			Definition.Action == EUEGT1SimActionType::Eat ? Model.GetPlayer().Needs.Hunger :
			Definition.Action == EUEGT1SimActionType::Hygiene ? Model.GetPlayer().Needs.Hygiene : Model.GetPlayer().Needs.Social;
		FString Result;
		TestTrue(TEXT("The activity completes through the shared simulation model"),
			Model.PerformPlayerActivity(Definition.Action, Venue->VenueId, Result));
		TestEqual(TEXT("The player records the completed activity"), Model.GetPlayer().LastAction, Definition.Action);
		TestEqual(TEXT("The player records the activity venue"), Model.GetPlayer().LastVenueId, Venue->VenueId);
		TestFalse(TEXT("The HUD receives a player activity result"), Model.GetPlayer().LastActivityMessage.IsEmpty());
		if (Definition.Action == EUEGT1SimActionType::Work)
		{
			TestTrue(TEXT("Player work earns the venue's hourly wage"), Model.GetPlayer().Money > MoneyBefore);
		}
		else if (Definition.Cost > 0.0f)
		{
			TestTrue(TEXT("Paid player activities deduct money"), Model.GetPlayer().Money < MoneyBefore);
		}
		else
		{
			const float NeedAfter = Definition.Action == EUEGT1SimActionType::Sleep ? Model.GetPlayer().Needs.Energy :
				Definition.Action == EUEGT1SimActionType::Hygiene ? Model.GetPlayer().Needs.Hygiene : Model.GetPlayer().Needs.Social;
			TestTrue(TEXT("Free restorative activities improve their matching player need"), NeedAfter > NeedBefore);
		}
	}

	FUEGT1TownSimulationModel CalendarModel = MakeModel(7319, 1);
	const FDateTime Start = CalendarModel.GetCalendarDateTime();
	CalendarModel.AdvanceMinutes(24.0f * 60.0f + 75.0f);
	const FDateTime Later = CalendarModel.GetCalendarDateTime();
	TestEqual(TEXT("The UI calendar starts on the configured date"), Start.GetYear(), Tuning.CalendarStartYear);
	TestEqual(TEXT("The UI date advances with simulation days"), Later.GetDay(), Start.GetDay() + 1);
	TestEqual(TEXT("The UI time advances with simulation minutes"), Later.GetHour(), 7);
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUEGT1NeedsAndRecoveryTest,
	"UEGT1.TownSimulation.NeedsDecayAndRecovery",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUEGT1NeedsAndRecoveryTest::RunTest(const FString& Parameters)
{
	FUEGT1TownSimulationModel Model = MakeModel(7319, 1);
	const FUEGT1NPCSimulationState Before = Model.GetNPCs()[0];
	Model.AdvanceMinutes(10.0f);
	const FUEGT1NPCSimulationState AfterDecay = Model.GetNPCs()[0];
	TestTrue(TEXT("Energy decays with simulation time"), AfterDecay.Needs.Energy < Before.Needs.Energy);
	TestTrue(TEXT("Hunger satisfaction decays with simulation time"), AfterDecay.Needs.Hunger < Before.Needs.Hunger);
	TestTrue(TEXT("Hygiene decays with simulation time"), AfterDecay.Needs.Hygiene < Before.Needs.Hygiene);
	TestTrue(TEXT("Social satisfaction decays with simulation time"), AfterDecay.Needs.Social < Before.Needs.Social);

	const FUEGT1SimulationTuning Tuning = UUEGT1TownSimulationSettings::Get().MakeTuning();
	const FUEGT1ActionDefinition* Eat = FindDefinition(Tuning, EUEGT1SimActionType::Eat, EUEGT1TownVenueType::FoodVenue);
	TestNotNull(TEXT("The data-driven paid meal action exists"), Eat);
	if (Eat)
	{
		FUEGT1NPCSimulationState NPC = AfterDecay;
		NPC.Money = 20.0f;
		const float HungerBefore = NPC.Needs.Hunger;
		FUEGT1SimulationMetrics Metrics;
		UEGT1TownSimulation::ApplyActionOutcome(NPC, *Eat, Metrics);
		TestTrue(TEXT("Eating restores hunger satisfaction"), NPC.Needs.Hunger > HungerBefore);
		TestEqual(TEXT("Eating records one completed action"), NPC.CompletedEatActions, 1);
	}
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUEGT1TownTransactionsTest,
	"UEGT1.TownSimulation.Transactions",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUEGT1TownTransactionsTest::RunTest(const FString& Parameters)
{
	const FUEGT1SimulationTuning Tuning = UUEGT1TownSimulationSettings::Get().MakeTuning();
	const FUEGT1ActionDefinition* Meal = FindDefinition(Tuning, EUEGT1SimActionType::Eat, EUEGT1TownVenueType::FoodVenue);
	const FUEGT1ActionDefinition* HomeMeal = FindDefinition(Tuning, EUEGT1SimActionType::Eat, EUEGT1TownVenueType::Home);
	const FUEGT1ActionDefinition* Work = FindDefinition(Tuning, EUEGT1SimActionType::Work, EUEGT1TownVenueType::Workplace);
	TestNotNull(TEXT("Paid meals are configured"), Meal);
	TestNotNull(TEXT("Home groceries are configured"), HomeMeal);
	TestNotNull(TEXT("Hourly work is configured"), Work);
	if (!Meal || !HomeMeal || !Work)
	{
		return false;
	}
	FUEGT1NPCSimulationState NPC;
	NPC.Money = 20.0f;
	FUEGT1SimulationMetrics Metrics;
	UEGT1TownSimulation::ApplyActionOutcome(NPC, *Meal, Metrics);
	TestEqual(TEXT("A meal debits its configured price"), NPC.Money, 20.0f - Meal->Cost);
	TestEqual(TEXT("Eating at home costs one dollar for groceries"), HomeMeal->Cost, 1.0f);
	FUEGT1ActionDefinition HourlyWork = *Work;
	HourlyWork.Earnings = 23.0f;
	UEGT1TownSimulation::ApplyActionOutcome(NPC, HourlyWork, Metrics);
	TestEqual(TEXT("One completed work interval credits one hourly wage"), NPC.Money, 20.0f - Meal->Cost + HourlyWork.Earnings);
	TestEqual(TEXT("Work decisions are made one hour at a time"), Work->DurationMinutes, 60.0f);
	TestEqual(TEXT("The generic work action has no fixed-shift lump payment"), Work->Earnings, 0.0f);
	TestEqual(TEXT("Spending is recorded"), Metrics.MoneySpent, Meal->Cost);
	TestEqual(TEXT("Hourly earnings are recorded"), Metrics.MoneyEarned, HourlyWork.Earnings);
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUEGT1AffordabilityAndReplanTest,
	"UEGT1.TownSimulation.AffordabilityAndReplan",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUEGT1AffordabilityAndReplanTest::RunTest(const FString& Parameters)
{
	FUEGT1TownSimulationModel Model = MakeModel(7319, 1);
	FUEGT1NPCSimulationState* NPC = Model.FindNPC(TEXT("Resident_001"));
	TestNotNull(TEXT("The deterministic resident exists"), NPC);
	if (!NPC)
	{
		return false;
	}
	NPC->Money = 0.0f;
	NPC->Needs = FUEGT1NeedState{ 0.9f, 0.05f, 0.9f, 0.9f };
	const FUEGT1ActionUtilityScore Choice = Model.EvaluateBestAction(0, true);
	const FUEGT1TownVenueState* ChoiceVenue = Model.GetVenues().FindByPredicate([&Choice](const FUEGT1TownVenueState& Venue)
	{
		return Venue.VenueId == Choice.DestinationId;
	});
	TestTrue(TEXT("A broke resident chooses work or a genuinely free alternative"), Choice.Action == EUEGT1SimActionType::Work ||
		(Choice.Action == EUEGT1SimActionType::Socialize && ChoiceVenue && ChoiceVenue->VenueType == EUEGT1TownVenueType::Park));
	TestFalse(TEXT("A broke resident never selects either paid meal"),
		Choice.Action == EUEGT1SimActionType::Eat);

	FUEGT1TownSimulationModel ClosedModel = MakeModel(7319, 1);
	FUEGT1NPCSimulationState* ClosedNPC = ClosedModel.FindNPC(TEXT("Resident_001"));
	for (const FUEGT1TownVenueState& Venue : ClosedModel.GetVenues())
	{
		if (Venue.VenueType == EUEGT1TownVenueType::FoodVenue)
		{
			FUEGT1TownVenueState* MutableVenue = ClosedModel.FindVenue(Venue.VenueId);
			MutableVenue->OpeningHour = 12.0f;
			MutableVenue->ClosingHour = 13.0f;
		}
	}
	if (ClosedNPC)
	{
		ClosedNPC->Money = 100.0f;
		ClosedNPC->Needs = FUEGT1NeedState{ 0.95f, 0.01f, 0.95f, 0.95f };
		ClosedModel.EvaluateBestAction(0, true);
		const bool bClosedWasRejected = ClosedNPC->UtilityScores.ContainsByPredicate([](const FUEGT1ActionUtilityScore& Utility)
		{
			return Utility.Action == EUEGT1SimActionType::Eat && Utility.Factors.Contains(TEXT("close"));
		});
		TestTrue(TEXT("Closed destinations are rejected with a visible reason"), bClosedWasRejected);
	}

	FUEGT1TownSimulationModel FullModel = MakeModel(7319, 1);
	FUEGT1NPCSimulationState* FullNPC = FullModel.FindNPC(TEXT("Resident_001"));
	for (const FUEGT1TownVenueState& Venue : FullModel.GetVenues())
	{
		if (Venue.VenueType == EUEGT1TownVenueType::FoodVenue)
		{
			FullModel.FindVenue(Venue.VenueId)->Capacity = 0;
		}
	}
	if (FullNPC)
	{
		FullNPC->Money = 100.0f;
		FullNPC->Needs = FUEGT1NeedState{ 0.95f, 0.01f, 0.95f, 0.95f };
		FullModel.EvaluateBestAction(0, true);
		const bool bFullWasRejected = FullNPC->UtilityScores.ContainsByPredicate([](const FUEGT1ActionUtilityScore& Utility)
		{
			return Utility.Action == EUEGT1SimActionType::Eat && Utility.Factors.Contains(TEXT("full"));
		});
		TestTrue(TEXT("Full destinations are rejected with a visible reason"), bFullWasRejected);
	}

	Model.AdvanceMinutes(5.0f);
	NPC = Model.FindNPC(TEXT("Resident_001"));
	const FName FirstDestination = NPC ? NPC->DestinationId : NAME_None;
	if (FUEGT1TownVenueState* Destination = Model.FindVenue(FirstDestination))
	{
		Destination->bReachable = false;
	}
	Model.AdvanceMinutes(5.0f);
	NPC = Model.FindNPC(TEXT("Resident_001"));
	TestTrue(TEXT("An unreachable destination records a failure and replans"),
		Model.GetMetrics().FailedDestinationCount > 0 && NPC && !NPC->LatestFailureReason.IsEmpty());
	TestTrue(TEXT("A failed destination does not permanently retain the resident"), NPC && NPC->DestinationId != FirstDestination);
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUEGT1SimulationPersistenceTest,
	"UEGT1.TownSimulation.PersistenceSnapshot",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUEGT1SimulationPersistenceTest::RunTest(const FString& Parameters)
{
	FUEGT1TownSimulationModel Original = MakeModel();
	Original.AdvanceMinutes(420.0f);
	const FUEGT1TownSimulationSnapshot Snapshot = Original.MakeSnapshot();
	UUEGT1TownSimulationSaveGame* SaveObject = NewObject<UUEGT1TownSimulationSaveGame>();
	SaveObject->Snapshot = Snapshot;
	TArray<uint8> SerializedData;
	TestTrue(TEXT("Unreal SaveGame serialization accepts the simulation snapshot"),
		UGameplayStatics::SaveGameToMemory(SaveObject, SerializedData));
	const UUEGT1TownSimulationSaveGame* LoadedObject = Cast<UUEGT1TownSimulationSaveGame>(
		UGameplayStatics::LoadGameFromMemory(SerializedData));
	TestNotNull(TEXT("Unreal SaveGame serialization restores the simulation object"), LoadedObject);
	FUEGT1TownSimulationModel Restored;
	TestTrue(TEXT("A meaningful serialized simulation snapshot restores"), LoadedObject &&
		Restored.Restore(LoadedObject->Snapshot, UUEGT1TownSimulationSettings::Get().MakeTuning()));
	TestEqual(TEXT("The town seed is restored"), Restored.GetSeed(), Original.GetSeed());
	TestTrue(TEXT("The clock is restored"), FMath::IsNearlyEqual(Restored.GetAbsoluteMinutes(), Original.GetAbsoluteMinutes()));
	TestEqual(TEXT("Every resident is restored"), Restored.GetNPCs().Num(), Original.GetNPCs().Num());
	TestEqual(TEXT("Every venue is restored"), Restored.GetVenues().Num(), Original.GetVenues().Num());
	TestTrue(TEXT("Player money is restored"), FMath::IsNearlyEqual(Restored.GetPlayer().Money, Original.GetPlayer().Money));
	TestTrue(TEXT("Player needs are restored"), FMath::IsNearlyEqual(Restored.GetPlayer().Needs.Hunger, Original.GetPlayer().Needs.Hunger));
	if (!Restored.GetNPCs().IsEmpty())
	{
		const FUEGT1NPCSimulationState& A = Original.GetNPCs()[0];
		const FUEGT1NPCSimulationState& B = Restored.GetNPCs()[0];
		TestEqual(TEXT("Resident action state is restored"), B.CurrentAction, A.CurrentAction);
		TestEqual(TEXT("Resident destination state is restored"), B.DestinationId, A.DestinationId);
		TestEqual(TEXT("Resident bed assignment is restored"), B.BedId, A.BedId);
		TestEqual(TEXT("Resident household relationships are restored"), B.HouseholdRelationships.Num(), A.HouseholdRelationships.Num());
		TestEqual(TEXT("Resident thought text is restored"), B.CurrentThought, A.CurrentThought);
		TestTrue(TEXT("Resident money is restored"), FMath::IsNearlyEqual(B.Money, A.Money));
		TestTrue(TEXT("Resident needs are restored"), FMath::IsNearlyEqual(B.Needs.Hunger, A.Needs.Hunger));
	}
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUEGT1TwoDayTownSimulationTest,
	"UEGT1.TownSimulation.TwoDaySmoke",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUEGT1TwoDayTownSimulationTest::RunTest(const FString& Parameters)
{
	FUEGT1TownSimulationModel Model = MakeModel(7319, 100);
	TestEqual(TEXT("The large-town simulation starts with one hundred residents"), Model.GetNPCs().Num(), 100);
	Model.AdvanceMinutes(2.0f * 24.0f * 60.0f);

	int32 SleepCount = 0;
	int32 EatCount = 0;
	int32 HygieneCount = 0;
	int32 SocialCount = 0;
	int32 WorkCount = 0;
	TSet<int32> ObservedWorkSessionLengths;
	bool bAllHaveHomes = true;
	bool bAllHaveBeds = true;
	bool bNoPermanentIdle = true;
	bool bHourlyWorkAccounting = true;
	bool bPersonalWorkLimitsRespected = true;
	for (const FUEGT1NPCSimulationState& NPC : Model.GetNPCs())
	{
		SleepCount += NPC.CompletedSleepActions;
		EatCount += NPC.CompletedEatActions;
		HygieneCount += NPC.CompletedHygieneActions;
		SocialCount += NPC.CompletedSocialActions;
		WorkCount += NPC.CompletedWorkActions;
		if (NPC.LongestWorkSessionMinutes > 0.0f)
		{
			ObservedWorkSessionLengths.Add(FMath::RoundToInt(NPC.LongestWorkSessionMinutes / 60.0f));
		}
		bHourlyWorkAccounting &= FMath::IsNearlyEqual(NPC.TotalWorkMinutes,
			static_cast<float>(NPC.CompletedWorkActions) * 60.0f);
		bPersonalWorkLimitsRespected &= NPC.LongestWorkSessionMinutes <= NPC.PreferredWorkSessionHours * 60.0f + KINDA_SMALL_NUMBER;
		bAllHaveHomes &= !NPC.HomeId.IsNone();
		bAllHaveBeds &= !NPC.BedId.IsNone();
		bAllHaveHomes &= NPC.HouseholdRelationships.Num() == 3 && !NPC.CurrentThought.IsEmpty();
		bNoPermanentIdle &= NPC.ConsecutiveIdleMinutes < 360.0f;
	}
	TestTrue(TEXT("Every resident has an assigned home"), bAllHaveHomes);
	TestTrue(TEXT("Every resident retains an assigned physical bed"), bAllHaveBeds && Model.HasCompleteBedAssignments());
	TestTrue(TEXT("Every household relationship remains complete through the two-day run"), Model.HasCompleteHouseholdRelationships());
	TestTrue(TEXT("Residents sleep during the two-day run"), SleepCount > 0);
	TestTrue(TEXT("Residents eat during the two-day run"), EatCount > 0);
	TestTrue(TEXT("Residents maintain hygiene during the two-day run"), HygieneCount > 0);
	TestTrue(TEXT("Residents socialize during the two-day run"), SocialCount > 0);
	TestTrue(TEXT("Residents work and earn wages during the two-day run"), WorkCount > 0 && Model.GetMetrics().MoneyEarned > 0.0f);
	TestTrue(TEXT("Work is paid and recorded in one-hour decisions"), bHourlyWorkAccounting);
	TestTrue(TEXT("Residents stop at their personal willing session length"), bPersonalWorkLimitsRespected);
	TestTrue(TEXT("Residents choose more than one work-session length"), ObservedWorkSessionLengths.Num() > 1);
	TestTrue(TEXT("Residents spend money during the two-day run"), Model.GetMetrics().MoneySpent > 0.0f);
	TestTrue(TEXT("Unaffordable choices produce work or free fallbacks"), Model.GetMetrics().AffordabilityFallbackCount > 0);
	TestTrue(TEXT("No resident remains permanently stuck in idle"), bNoPermanentIdle);
	return !HasAnyErrors();
}

#endif
