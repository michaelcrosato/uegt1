#include "Simulation/UEGT1TownSimulationModel.h"

#include "Algo/Sort.h"
#include "World/UEGT1TownGeneration.h"

namespace
{
	float GetNeedUrgency(const FUEGT1NPCSimulationState& NPC, EUEGT1SimActionType Action)
	{
		switch (Action)
		{
		case EUEGT1SimActionType::Sleep: return 1.0f - NPC.Needs.Energy;
		case EUEGT1SimActionType::Eat: return 1.0f - NPC.Needs.Hunger;
		case EUEGT1SimActionType::Hygiene: return 1.0f - NPC.Needs.Hygiene;
		case EUEGT1SimActionType::Socialize: return 1.0f - NPC.Needs.Social;
		default: return 0.0f;
		}
	}

	float SumPositiveEffects(const FUEGT1NeedState& Effects)
	{
		return FMath::Max(0.0f, Effects.Energy) + FMath::Max(0.0f, Effects.Hunger) +
			FMath::Max(0.0f, Effects.Hygiene) + FMath::Max(0.0f, Effects.Social);
	}

	const TCHAR* ResidentNames[] = {
		TEXT("Avery"), TEXT("Briar"), TEXT("Cass"), TEXT("Dara"), TEXT("Emery"), TEXT("Finch"), TEXT("Gray"),
		TEXT("Hollis"), TEXT("Indra"), TEXT("Jules"), TEXT("Kai"), TEXT("Lane"), TEXT("Maren"), TEXT("Noel"),
		TEXT("Oren"), TEXT("Pax"), TEXT("Quinn"), TEXT("Remy"), TEXT("Sage"), TEXT("Tavi"), TEXT("Uma"),
		TEXT("Vale"), TEXT("Wren"), TEXT("Xan"), TEXT("Yael"), TEXT("Zuri")
	};

	int32 CountBeds(const TArray<FUEGT1TownVenueState>& Venues)
	{
		int32 Count = 0;
		for (const FUEGT1TownVenueState& Venue : Venues)
		{
			if (Venue.VenueType == EUEGT1TownVenueType::Home)
			{
				Count += Venue.Beds.Num();
			}
		}
		return Count;
	}

	bool HasValidBedAssignments(const TArray<FUEGT1TownVenueState>& Venues,
		const TArray<FUEGT1NPCSimulationState>& NPCs)
	{
		TMap<FName, FName> BedHomes;
		for (const FUEGT1TownVenueState& Venue : Venues)
		{
			if (Venue.VenueType != EUEGT1TownVenueType::Home)
			{
				continue;
			}
			for (const FUEGT1TownBedState& Bed : Venue.Beds)
			{
				if (Bed.BedId.IsNone() || BedHomes.Contains(Bed.BedId))
				{
					return false;
				}
				BedHomes.Add(Bed.BedId, Venue.VenueId);
			}
		}

		TSet<FName> AssignedBeds;
		for (const FUEGT1NPCSimulationState& NPC : NPCs)
		{
			const FName* BedHome = BedHomes.Find(NPC.BedId);
			if (!BedHome || *BedHome != NPC.HomeId || AssignedBeds.Contains(NPC.BedId))
			{
				return false;
			}
			AssignedBeds.Add(NPC.BedId);
		}
		return AssignedBeds.Num() == NPCs.Num();
	}

	void AssignHouseholdRelationships(TArray<FUEGT1NPCSimulationState>& NPCs)
	{
		TMap<FName, TArray<int32>> HouseholdMembers;
		for (int32 NPCIndex = 0; NPCIndex < NPCs.Num(); ++NPCIndex)
		{
			NPCs[NPCIndex].HouseholdRelationships.Reset();
			HouseholdMembers.FindOrAdd(NPCs[NPCIndex].HomeId).Add(NPCIndex);
		}

		TArray<FName> HomeIds;
		HouseholdMembers.GetKeys(HomeIds);
		HomeIds.Sort([](const FName& A, const FName& B) { return A.LexicalLess(B); });
		for (int32 HomeIndex = 0; HomeIndex < HomeIds.Num(); ++HomeIndex)
		{
			const EUEGT1HouseholdRelationship Relationship = static_cast<EUEGT1HouseholdRelationship>(HomeIndex % 3);
			const TArray<int32>& Members = HouseholdMembers.FindChecked(HomeIds[HomeIndex]);
			for (int32 MemberIndex = 0; MemberIndex < Members.Num(); ++MemberIndex)
			{
				for (int32 OtherIndex = MemberIndex + 1; OtherIndex < Members.Num(); ++OtherIndex)
				{
					FUEGT1HouseholdRelationshipState ToOther;
					ToOther.OtherNpcId = NPCs[Members[OtherIndex]].NpcId;
					ToOther.Relationship = Relationship;
					NPCs[Members[MemberIndex]].HouseholdRelationships.Add(ToOther);

					FUEGT1HouseholdRelationshipState ToMember;
					ToMember.OtherNpcId = NPCs[Members[MemberIndex]].NpcId;
					ToMember.Relationship = Relationship;
					NPCs[Members[OtherIndex]].HouseholdRelationships.Add(ToMember);
				}
			}
		}
	}

	bool HasValidHouseholdRelationships(const TArray<FUEGT1NPCSimulationState>& NPCs)
	{
		for (const FUEGT1NPCSimulationState& NPC : NPCs)
		{
			TSet<FName> SeenHousemates;
			int32 ExpectedHousemates = 0;
			for (const FUEGT1NPCSimulationState& Other : NPCs)
			{
				if (Other.NpcId != NPC.NpcId && Other.HomeId == NPC.HomeId)
				{
					++ExpectedHousemates;
				}
			}
			for (const FUEGT1HouseholdRelationshipState& Relationship : NPC.HouseholdRelationships)
			{
				const FUEGT1NPCSimulationState* Other = NPCs.FindByPredicate([&Relationship](const FUEGT1NPCSimulationState& Candidate)
				{
					return Candidate.NpcId == Relationship.OtherNpcId;
				});
				if (!Other || Other->HomeId != NPC.HomeId || Other->NpcId == NPC.NpcId || SeenHousemates.Contains(Other->NpcId))
				{
					return false;
				}
				const FUEGT1HouseholdRelationshipState* Reverse = Other->HouseholdRelationships.FindByPredicate(
					[&NPC](const FUEGT1HouseholdRelationshipState& Candidate) { return Candidate.OtherNpcId == NPC.NpcId; });
				if (!Reverse || Reverse->Relationship != Relationship.Relationship)
				{
					return false;
				}
				SeenHousemates.Add(Other->NpcId);
			}
			if (SeenHousemates.Num() != ExpectedHousemates)
			{
				return false;
			}
		}
		return true;
	}
}

void FUEGT1TownSimulationModel::Initialize(int32 InSeed, int32 NPCCount, const TArray<FUEGT1TownVenueState>& InVenues,
	const FUEGT1SimulationTuning& InTuning)
{
	Seed = InSeed;
	Tuning = InTuning;
	AbsoluteMinutes = FMath::Clamp(Tuning.StartHour, 0.0f, 24.0f) * 60.0f;
	Venues = InVenues;
	NPCs.Reset();
	Metrics = FUEGT1SimulationMetrics();
	Player = FUEGT1PlayerSimulationState();
	Player.Money = Tuning.PlayerStartingMoney;
	Player.LastActivityMessage = TEXT("Explore the town and choose an activity.");
	for (FUEGT1TownVenueState& Venue : Venues)
	{
		Venue.Reservations.Reset();
	}

	TArray<TPair<const FUEGT1TownVenueState*, const FUEGT1TownBedState*>> AvailableBeds;
	TArray<const FUEGT1TownVenueState*> Workplaces;
	for (const FUEGT1TownVenueState& Venue : Venues)
	{
		if (Venue.VenueType == EUEGT1TownVenueType::Home)
		{
			for (const FUEGT1TownBedState& Bed : Venue.Beds)
			{
				AvailableBeds.Emplace(&Venue, &Bed);
			}
		}
		else if (Venue.VenueType == EUEGT1TownVenueType::Workplace)
		{
			Workplaces.Add(&Venue);
		}
	}
	Workplaces.Sort([](const FUEGT1TownVenueState& A, const FUEGT1TownVenueState& B)
	{
		return A.VenueId.LexicalLess(B.VenueId);
	});
	NPCCount = FMath::Max(NPCCount, 1);
	if (AvailableBeds.Num() < NPCCount)
	{
		bInitialized = false;
		return;
	}
	AvailableBeds.Sort([](const TPair<const FUEGT1TownVenueState*, const FUEGT1TownBedState*>& A,
		const TPair<const FUEGT1TownVenueState*, const FUEGT1TownBedState*>& B)
	{
		return A.Value->BedId.LexicalLess(B.Value->BedId);
	});

	FRandomStream Random(InSeed + 991);
	for (int32 Index = 0; Index < NPCCount; ++Index)
	{
		FUEGT1NPCSimulationState NPC;
		NPC.NpcId = FName(*FString::Printf(TEXT("Resident_%03d"), Index + 1));
		NPC.DisplayName = ResidentNames[Index % UE_ARRAY_COUNT(ResidentNames)];
		if (Index >= UE_ARRAY_COUNT(ResidentNames))
		{
			NPC.DisplayName += FString::Printf(TEXT(" %d"), Index / UE_ARRAY_COUNT(ResidentNames) + 1);
		}
		const FUEGT1TownVenueState& Home = *AvailableBeds[Index].Key;
		const FUEGT1TownBedState& Bed = *AvailableBeds[Index].Value;
		NPC.HomeId = Home.VenueId;
		NPC.BedId = Bed.BedId;
		NPC.CurrentVenueId = Home.VenueId;
		NPC.WorldLocation = Bed.WorldLocation + FVector(0.0f, 0.0f, 92.0f);
		if (!Workplaces.IsEmpty())
		{
			const FUEGT1TownVenueState& PreferredJob = *Workplaces[Index % Workplaces.Num()];
			NPC.JobId = PreferredJob.VenueId;
			NPC.Schedule.WorkStartHour = PreferredJob.OpeningHour;
			NPC.Schedule.WorkEndHour = PreferredJob.ClosingHour;
			NPC.Schedule.WorkdayMask = 0x7f;
			if (PreferredJob.OpeningHour > PreferredJob.ClosingHour)
			{
				NPC.Schedule.SleepStartHour = 7.0f + 0.25f * static_cast<float>(Index % 3);
				NPC.Schedule.SleepEndHour = 14.0f + 0.25f * static_cast<float>(Index % 3);
			}
		}
		NPC.Money = (Index % 5) == 0 ? 0.0f : Random.FRandRange(Tuning.StartingMoneyMinimum, Tuning.StartingMoneyMaximum);
		NPC.Needs.Energy = Random.FRandRange(0.24f, 0.82f);
		NPC.Needs.Hunger = Random.FRandRange(0.18f, 0.80f);
		NPC.Needs.Hygiene = Random.FRandRange(0.24f, 0.84f);
		NPC.Needs.Social = Random.FRandRange(0.20f, 0.82f);
		if (NPC.Schedule.SleepStartHour == 22.0f && NPC.Schedule.SleepEndHour == 6.0f)
		{
			NPC.Schedule.SleepStartHour = 21.5f + 0.5f * static_cast<float>(Index % 3);
			NPC.Schedule.SleepEndHour = 5.5f + 0.5f * static_cast<float>(Index % 3);
		}
		NPC.SavingsGoal = Random.FRandRange(35.0f, 90.0f);
		NPC.WorkDrive = Random.FRandRange(0.75f, 1.25f);
		NPC.PreferredWorkSessionHours = static_cast<float>(Random.RandRange(1, 6));
		NPC.CurrentThought = TEXT("Getting ready for the day at home.");
		NPCs.Add(MoveTemp(NPC));
	}
	AssignHouseholdRelationships(NPCs);
	bInitialized = HasValidBedAssignments(Venues, NPCs) && HasValidHouseholdRelationships(NPCs);
}

bool FUEGT1TownSimulationModel::Restore(const FUEGT1TownSimulationSnapshot& Snapshot, const FUEGT1SimulationTuning& InTuning)
{
	if (Snapshot.Version != 4 || Snapshot.Seed == 0 || Snapshot.Venues.IsEmpty() || Snapshot.NPCs.IsEmpty() ||
		!HasValidBedAssignments(Snapshot.Venues, Snapshot.NPCs) || !HasValidHouseholdRelationships(Snapshot.NPCs))
	{
		return false;
	}
	Seed = Snapshot.Seed;
	AbsoluteMinutes = Snapshot.AbsoluteMinutes;
	Venues = Snapshot.Venues;
	NPCs = Snapshot.NPCs;
	Metrics = Snapshot.Metrics;
	Player = Snapshot.Player;
	Tuning = InTuning;
	bInitialized = true;
	return true;
}

FUEGT1TownSimulationSnapshot FUEGT1TownSimulationModel::MakeSnapshot() const
{
	FUEGT1TownSimulationSnapshot Snapshot;
	Snapshot.Seed = Seed;
	Snapshot.AbsoluteMinutes = AbsoluteMinutes;
	Snapshot.Venues = Venues;
	Snapshot.NPCs = NPCs;
	Snapshot.Metrics = Metrics;
	Snapshot.Player = Player;
	return Snapshot;
}

FDateTime FUEGT1TownSimulationModel::GetCalendarDateTime() const
{
	const int32 Year = FMath::Clamp(Tuning.CalendarStartYear, 1, 9999);
	const int32 Month = FMath::Clamp(Tuning.CalendarStartMonth, 1, 12);
	const int32 Day = FMath::Clamp(Tuning.CalendarStartDay, 1, FDateTime::DaysInMonth(Year, Month));
	return FDateTime(Year, Month, Day) + FTimespan::FromMinutes(AbsoluteMinutes);
}

int32 FUEGT1TownSimulationModel::GetBedCount() const
{
	return CountBeds(Venues);
}

bool FUEGT1TownSimulationModel::HasCompleteBedAssignments() const
{
	return HasValidBedAssignments(Venues, NPCs);
}

bool FUEGT1TownSimulationModel::HasCompleteHouseholdRelationships() const
{
	return HasValidHouseholdRelationships(NPCs);
}

void FUEGT1TownSimulationModel::AdvanceMinutes(float Minutes)
{
	if (!bInitialized || Minutes <= 0.0f)
	{
		return;
	}
	float Remaining = Minutes;
	const float StepSize = FMath::Max(Tuning.SimulationStepMinutes, 1.0f);
	while (Remaining > KINDA_SMALL_NUMBER)
	{
		const float Step = FMath::Min(Remaining, StepSize);
		AdvanceStep(Step);
		Remaining -= Step;
	}
}

void FUEGT1TownSimulationModel::AdvanceStep(float Minutes)
{
	AbsoluteMinutes += Minutes;
	const float PlayerHours = Minutes / 60.0f;
	Player.Needs.Energy -= Tuning.EnergyDecayPerHour * PlayerHours;
	Player.Needs.Hunger -= Tuning.HungerDecayPerHour * PlayerHours;
	Player.Needs.Hygiene -= Tuning.HygieneDecayPerHour * PlayerHours;
	Player.Needs.Social -= Tuning.SocialDecayPerHour * PlayerHours;
	Player.Needs.Clamp();
	for (FUEGT1NPCSimulationState& NPC : NPCs)
	{
		DecayNeeds(NPC, Minutes);
		if (NPC.CurrentAction == EUEGT1SimActionType::Travel)
		{
			AdvanceTravel(NPC, Minutes);
		}
		else if (NPC.RemainingActionMinutes > 0.0f)
		{
			NPC.RemainingActionMinutes -= Minutes;
			if (NPC.CurrentAction == EUEGT1SimActionType::Idle)
			{
				NPC.ConsecutiveIdleMinutes += Minutes;
			}
			if (NPC.RemainingActionMinutes <= KINDA_SMALL_NUMBER)
			{
				if (NPC.CurrentAction == EUEGT1SimActionType::Idle)
				{
					NPC.RemainingActionMinutes = 0.0f;
					PlanNPC(NPC);
				}
				else
				{
					CompleteAction(NPC);
				}
			}
		}
		else
		{
			PlanNPC(NPC);
		}
	}
}

void FUEGT1TownSimulationModel::DecayNeeds(FUEGT1NPCSimulationState& NPC, float Minutes) const
{
	const float Hours = Minutes / 60.0f;
	NPC.Needs.Energy -= Tuning.EnergyDecayPerHour * Hours;
	NPC.Needs.Hunger -= Tuning.HungerDecayPerHour * Hours;
	NPC.Needs.Hygiene -= Tuning.HygieneDecayPerHour * Hours;
	NPC.Needs.Social -= Tuning.SocialDecayPerHour * Hours;
	NPC.Needs.Clamp();
}

FUEGT1ActionUtilityScore FUEGT1TownSimulationModel::EvaluateBestAction(int32 NPCIndex, bool bCommitUtilityScores)
{
	FUEGT1ActionUtilityScore Best;
	if (!NPCs.IsValidIndex(NPCIndex))
	{
		return Best;
	}
	FUEGT1NPCSimulationState& NPC = NPCs[NPCIndex];
	TArray<FUEGT1ActionUtilityScore> Scores;
	for (const FUEGT1ActionDefinition& Definition : Tuning.ActionDefinitions)
	{
		bool bUnaffordable = false;
		Scores.Add(EvaluateDefinition(NPC, Definition, bUnaffordable));
	}
	Scores.Sort([](const FUEGT1ActionUtilityScore& A, const FUEGT1ActionUtilityScore& B)
	{
		return A.Score > B.Score;
	});
	if (!Scores.IsEmpty())
	{
		Best = Scores[0];
	}
	if (bCommitUtilityScores)
	{
		NPC.UtilityScores = MoveTemp(Scores);
	}
	return Best;
}

FUEGT1ActionUtilityScore FUEGT1TownSimulationModel::EvaluateDefinition(const FUEGT1NPCSimulationState& NPC,
	const FUEGT1ActionDefinition& Definition, bool& bOutUnaffordable) const
{
	bOutUnaffordable = NPC.Money + KINDA_SMALL_NUMBER < Definition.Cost;
	FUEGT1ActionUtilityScore Result;
	Result.Action = Definition.Action;
	if (bOutUnaffordable)
	{
		Result.Factors = FString::Printf(TEXT("unaffordable: $%.0f needed, $%.0f held"), Definition.Cost, NPC.Money);
		return Result;
	}
	if (Definition.Action == EUEGT1SimActionType::Work && NPC.CurrentWorkSessionMinutes > 0.0f &&
		NPC.CurrentWorkSessionMinutes + KINDA_SMALL_NUMBER >= NPC.PreferredWorkSessionHours * 60.0f)
	{
		Result.Factors = FString::Printf(TEXT("personal work limit reached after %.0f hours; taking a break"),
			NPC.CurrentWorkSessionMinutes / 60.0f);
		return Result;
	}

	const float Hour = GetHourOfDay();
	const float NeedUrgency = GetNeedUrgency(NPC, Definition.Action);
	const float PositiveBenefit = SumPositiveEffects(Definition.NeedEffects);
	const float BaseScore = Definition.BaseUtility + NeedUrgency * Definition.NeedWeight + PositiveBenefit * 18.0f -
		Definition.Cost * 0.75f;
	FString Rejection = TEXT("no registered destination");
	for (const FUEGT1TownVenueState& Venue : Venues)
	{
		if (Venue.VenueType != Definition.VenueType)
		{
			continue;
		}
		if (Definition.bPreferAssignedDestination && Definition.Action != EUEGT1SimActionType::Work)
		{
			if (!NPC.HomeId.IsNone() && Venue.VenueId != NPC.HomeId)
			{
				continue;
			}
		}
		if (!Venue.bReachable)
		{
			Rejection = TEXT("destination unreachable");
			continue;
		}
		if (!Venue.HasCapacityFor(NPC.NpcId))
		{
			Rejection = TEXT("destination full");
			continue;
		}
		const float Distance = CalculateTravelDistance(NPC, Venue, Definition.Action);
		const float ArrivalHour = FMath::Fmod(Hour + (Distance / FMath::Max(Tuning.GetTravelSpeedCentimetersPerSimulationMinute(), 1.0f)) / 60.0f, 24.0f);
		const float RequiredDuration = Definition.Action == EUEGT1SimActionType::Work
			? Tuning.WorkDecisionIntervalMinutes : Definition.DurationMinutes;
		if (!Venue.IsOpenForDuration(ArrivalHour, RequiredDuration))
		{
			Rejection = TEXT("destination closes before the action can finish");
			continue;
		}

		float Score = BaseScore - (Distance / 100.0f) * Tuning.TravelUtilityCostPerMeter;
		if (Definition.Action == EUEGT1SimActionType::Sleep)
		{
			Score += NPC.Schedule.IsSleepTime(Hour) ? 55.0f : 0.0f;
			Score += NPC.Needs.Energy < Tuning.CriticalNeedThreshold ? 70.0f : 0.0f;
		}
		else if (Definition.Action == EUEGT1SimActionType::Work)
		{
			const float SavingsGoal = FMath::Max(NPC.SavingsGoal, 1.0f);
			const float MoneyNeed = FMath::Clamp((SavingsGoal - NPC.Money) / SavingsGoal, 0.0f, 1.0f);
			const float HighestPersonalUrgency = FMath::Max(
				FMath::Max(1.0f - NPC.Needs.Energy, 1.0f - NPC.Needs.Hunger),
				FMath::Max(1.0f - NPC.Needs.Hygiene, 1.0f - NPC.Needs.Social));
			Score += MoneyNeed * 62.0f * NPC.WorkDrive;
			Score += NPC.Money < 1.0f ? 85.0f : (NPC.Money < Tuning.BrokeThreshold ? 42.0f : 0.0f);
			Score += Venue.HourlyRate * (0.35f + MoneyNeed * 0.65f);
			Score -= HighestPersonalUrgency * 34.0f;
			if (Venue.VenueId == NPC.JobId)
			{
				Score += 18.0f;
			}
			if (NPC.CurrentWorkSessionMinutes > 0.0f)
			{
				Score += Venue.VenueId == NPC.CurrentWorkVenueId ? 10.0f : -12.0f;
				Score -= (NPC.CurrentWorkSessionMinutes / 60.0f) * 8.0f;
			}
			if (NPC.Needs.Energy < Tuning.CriticalNeedThreshold || NPC.Needs.Hunger < Tuning.CriticalNeedThreshold)
			{
				Score -= 85.0f;
			}
		}
		else if (Definition.Action == EUEGT1SimActionType::Socialize && Hour >= 17.0f)
		{
			Score += 12.0f;
		}

		if (Score > Result.Score)
		{
			Result.DestinationId = Venue.VenueId;
			Result.Score = Score;
			if (Definition.Action == EUEGT1SimActionType::Work)
			{
				Result.Factors = FString::Printf(TEXT("money goal $%.0f, $%.0f/hr, travel %.0fm, worked %.0fh"),
					NPC.SavingsGoal, Venue.HourlyRate, Distance / 100.0f, NPC.CurrentWorkSessionMinutes / 60.0f);
			}
			else
			{
				Result.Factors = FString::Printf(TEXT("need %.2f, travel %.0fm, cost $%.0f, benefit %.2f"),
					NeedUrgency, Distance / 100.0f, Definition.Cost, PositiveBenefit);
			}
		}
	}
	if (Result.DestinationId.IsNone())
	{
		Result.Factors = Rejection;
	}
	return Result;
}

void FUEGT1TownSimulationModel::PlanNPC(FUEGT1NPCSimulationState& NPC)
{
	const int32 NPCIndex = static_cast<int32>(&NPC - NPCs.GetData());
	bool bSawUnaffordable = false;
	for (const FUEGT1ActionDefinition& Definition : Tuning.ActionDefinitions)
	{
		if (Definition.Cost > 0.0f && NPC.Money + KINDA_SMALL_NUMBER < Definition.Cost)
		{
			bSawUnaffordable = true;
		}
	}
	const FUEGT1ActionUtilityScore Best = EvaluateBestAction(NPCIndex, true);
	if (Best.Score <= 8.0f || Best.DestinationId.IsNone())
	{
		NPC.CurrentWorkSessionMinutes = 0.0f;
		NPC.CurrentWorkVenueId = NAME_None;
		NPC.CurrentAction = EUEGT1SimActionType::Idle;
		NPC.PlannedAction = EUEGT1SimActionType::Idle;
		NPC.RemainingActionMinutes = Tuning.ReplanIntervalMinutes;
		NPC.LatestFailureReason = Best.Factors.IsEmpty() ? TEXT("No useful action; waiting before replan") : Best.Factors;
		++Metrics.ReplanCount;
		UpdateThought(NPC);
		return;
	}

	FUEGT1TownVenueState* Destination = FindVenue(Best.DestinationId);
	if (!Destination || !Destination->bReachable || !Destination->HasCapacityFor(NPC.NpcId))
	{
		FailAndReplan(NPC, Destination ? TEXT("Selected destination became unavailable") : TEXT("Selected destination disappeared"));
		return;
	}
	Reserve(*Destination, NPC.NpcId);
	if (Best.Action == EUEGT1SimActionType::Work)
	{
		if (NPC.CurrentWorkVenueId != Destination->VenueId)
		{
			NPC.CurrentWorkSessionMinutes = 0.0f;
		}
		NPC.CurrentWorkVenueId = Destination->VenueId;
	}
	else
	{
		NPC.CurrentWorkSessionMinutes = 0.0f;
		NPC.CurrentWorkVenueId = NAME_None;
	}
	NPC.DestinationId = Destination->VenueId;
	NPC.PlannedAction = Best.Action;
	NPC.TravelPath = BuildTravelPath(NPC, *Destination, Best.Action);
	NPC.TravelPathIndex = NPC.TravelPath.Num() > 1 ? 1 : 0;
	NPC.CurrentVenueId = NAME_None;
	if (bSawUnaffordable && (Best.Action == EUEGT1SimActionType::Work ||
		(Best.Action == EUEGT1SimActionType::Eat && Destination->VenueType == EUEGT1TownVenueType::Home) ||
		(Best.Action == EUEGT1SimActionType::Socialize && Destination->VenueType == EUEGT1TownVenueType::Park)))
	{
		++Metrics.AffordabilityFallbackCount;
	}
	const FVector ActivityLocation = GetActivityLocation(NPC, *Destination, Best.Action);
	if (NPC.TravelPath.Num() <= 1 || FVector::DistSquared(NPC.WorldLocation, ActivityLocation + FVector(0.0f, 0.0f, 92.0f)) < 100.0f)
	{
		NPC.WorldLocation = ActivityLocation + FVector(0.0f, 0.0f, 92.0f);
		NPC.CurrentVenueId = Destination->VenueId;
		NPC.CurrentAction = Best.Action;
		const FUEGT1ActionDefinition* Definition = FindActionDefinition(Best.Action, Destination->VenueType);
		NPC.RemainingActionMinutes = Definition
			? (Best.Action == EUEGT1SimActionType::Work ? Tuning.WorkDecisionIntervalMinutes : Definition->DurationMinutes)
			: Tuning.ReplanIntervalMinutes;
	}
	else
	{
		NPC.CurrentAction = EUEGT1SimActionType::Travel;
		NPC.RemainingActionMinutes = CalculateTravelDistance(NPC, *Destination, Best.Action) /
			FMath::Max(Tuning.GetTravelSpeedCentimetersPerSimulationMinute(), 1.0f);
	}
	NPC.ConsecutiveIdleMinutes = 0.0f;
	UpdateThought(NPC, Destination);
}

void FUEGT1TownSimulationModel::AdvanceTravel(FUEGT1NPCSimulationState& NPC, float Minutes)
{
	FUEGT1TownVenueState* Destination = FindVenue(NPC.DestinationId);
	if (!Destination || !Destination->bReachable || !Destination->HasCapacityFor(NPC.NpcId))
	{
		FailAndReplan(NPC, TEXT("Travel failed: destination unavailable, full, or unreachable"));
		return;
	}

	float DistanceBudget = Tuning.GetTravelSpeedCentimetersPerSimulationMinute() * Minutes;
	while (DistanceBudget > KINDA_SMALL_NUMBER && NPC.TravelPath.IsValidIndex(NPC.TravelPathIndex))
	{
		const FVector Target = NPC.TravelPath[NPC.TravelPathIndex] + FVector(0.0f, 0.0f, 92.0f);
		const float Distance = FVector::Distance(NPC.WorldLocation, Target);
		if (Distance <= DistanceBudget + KINDA_SMALL_NUMBER)
		{
			NPC.WorldLocation = Target;
			DistanceBudget -= Distance;
			++NPC.TravelPathIndex;
		}
		else
		{
			NPC.WorldLocation += (Target - NPC.WorldLocation).GetSafeNormal() * DistanceBudget;
			DistanceBudget = 0.0f;
		}
	}
	NPC.RemainingActionMinutes = FMath::Max(0.0f, NPC.RemainingActionMinutes - Minutes);
	if (!NPC.TravelPath.IsValidIndex(NPC.TravelPathIndex))
	{
		const FUEGT1ActionDefinition* Definition = FindActionDefinition(NPC.PlannedAction, Destination->VenueType);
		const float RequiredDuration = NPC.PlannedAction == EUEGT1SimActionType::Work
			? Tuning.WorkDecisionIntervalMinutes : (Definition ? Definition->DurationMinutes : 0.0f);
		if (!Destination->IsOpenForDuration(GetHourOfDay(), RequiredDuration))
		{
			FailAndReplan(NPC, FString::Printf(TEXT("Arrived at %s too late to finish"), *Destination->VenueId.ToString()));
			return;
		}
		NPC.WorldLocation = GetActivityLocation(NPC, *Destination, NPC.PlannedAction) + FVector(0.0f, 0.0f, 92.0f);
		NPC.CurrentVenueId = Destination->VenueId;
		NPC.CurrentAction = NPC.PlannedAction;
		if (!Definition)
		{
			FailAndReplan(NPC, TEXT("Action definition missing on arrival"));
			return;
		}
		NPC.RemainingActionMinutes = NPC.PlannedAction == EUEGT1SimActionType::Work
			? Tuning.WorkDecisionIntervalMinutes : Definition->DurationMinutes;
		UpdateThought(NPC, Destination);
	}
}

void FUEGT1TownSimulationModel::CompleteAction(FUEGT1NPCSimulationState& NPC)
{
	const FUEGT1TownVenueState* Venue = FindVenue(NPC.DestinationId);
	const FUEGT1ActionDefinition* Definition = Venue ? FindActionDefinition(NPC.CurrentAction, Venue->VenueType) : nullptr;
	if (!Definition)
	{
		FailAndReplan(NPC, TEXT("Could not resolve completed action"));
		return;
	}
	if (NPC.Money + KINDA_SMALL_NUMBER < Definition->Cost)
	{
		FailAndReplan(NPC, TEXT("Funds changed before payment; replanning"));
		return;
	}
	FUEGT1ActionDefinition Outcome = *Definition;
	if (Definition->Action == EUEGT1SimActionType::Work)
	{
		const float PaidMinutes = Tuning.WorkDecisionIntervalMinutes;
		Outcome.Earnings = Venue->HourlyRate * PaidMinutes / 60.0f;
		NPC.CurrentWorkSessionMinutes += PaidMinutes;
		NPC.TotalWorkMinutes += PaidMinutes;
		NPC.LongestWorkSessionMinutes = FMath::Max(NPC.LongestWorkSessionMinutes, NPC.CurrentWorkSessionMinutes);
		NPC.CurrentWorkVenueId = Venue->VenueId;
	}
	UEGT1TownSimulation::ApplyActionOutcome(NPC, Outcome, Metrics);
	ReleaseReservation(NPC.DestinationId, NPC.NpcId);
	NPC.CurrentVenueId = NPC.DestinationId;
	NPC.DestinationId = NAME_None;
	NPC.CurrentAction = EUEGT1SimActionType::Idle;
	NPC.PlannedAction = EUEGT1SimActionType::Idle;
	NPC.RemainingActionMinutes = 0.0f;
	NPC.TravelPath.Reset();
	NPC.TravelPathIndex = 0;
	UpdateThought(NPC, Venue);
}

void FUEGT1TownSimulationModel::FailAndReplan(FUEGT1NPCSimulationState& NPC, const FString& Reason)
{
	ReleaseReservation(NPC.DestinationId, NPC.NpcId);
	NPC.LatestFailureReason = Reason;
	NPC.DestinationId = NAME_None;
	NPC.CurrentAction = EUEGT1SimActionType::Idle;
	NPC.PlannedAction = EUEGT1SimActionType::Idle;
	NPC.RemainingActionMinutes = 0.0f;
	NPC.TravelPath.Reset();
	NPC.TravelPathIndex = 0;
	NPC.CurrentWorkSessionMinutes = 0.0f;
	NPC.CurrentWorkVenueId = NAME_None;
	++Metrics.ReplanCount;
	++Metrics.FailedDestinationCount;
	PlanNPC(NPC);
}

void FUEGT1TownSimulationModel::UpdateThought(FUEGT1NPCSimulationState& NPC, const FUEGT1TownVenueState* Venue) const
{
	if (!Venue)
	{
		Venue = FindVenue(!NPC.DestinationId.IsNone() ? NPC.DestinationId : NPC.CurrentVenueId);
	}
	const FString VenueName = Venue && !Venue->DisplayName.IsEmpty() ? Venue->DisplayName :
		(Venue ? Venue->VenueId.ToString() : FString(TEXT("somewhere nearby")));
	const TCHAR* Household = TEXT("housemates");
	if (!NPC.HouseholdRelationships.IsEmpty())
	{
		switch (NPC.HouseholdRelationships[0].Relationship)
		{
		case EUEGT1HouseholdRelationship::Family: Household = TEXT("family"); break;
		case EUEGT1HouseholdRelationship::Friend: Household = TEXT("friends"); break;
		case EUEGT1HouseholdRelationship::Roommate: Household = TEXT("roommates"); break;
		default: break;
		}
	}

	const EUEGT1SimActionType ThoughtAction = NPC.CurrentAction == EUEGT1SimActionType::Travel
		? NPC.PlannedAction : NPC.CurrentAction;
	const bool bTraveling = NPC.CurrentAction == EUEGT1SimActionType::Travel;
	switch (ThoughtAction)
	{
	case EUEGT1SimActionType::Sleep:
		NPC.CurrentThought = bTraveling
			? FString::Printf(TEXT("I'm tired. Heading home to my %s."), Household)
			: TEXT("Finally, some sleep.");
		break;
	case EUEGT1SimActionType::Eat:
		if (Venue && Venue->VenueType == EUEGT1TownVenueType::Home)
		{
			NPC.CurrentThought = bTraveling ? TEXT("I'll cook at home. Groceries cost $1.") : TEXT("A $1 home meal hits the spot.");
		}
		else
		{
			NPC.CurrentThought = bTraveling ? FString::Printf(TEXT("I'm hungry. Going to %s."), *VenueName) : TEXT("Taking time to enjoy this meal.");
		}
		break;
	case EUEGT1SimActionType::Hygiene:
		NPC.CurrentThought = bTraveling ? TEXT("I need to freshen up at home.") : TEXT("A little self-care feels good.");
		break;
	case EUEGT1SimActionType::Socialize:
		NPC.CurrentThought = bTraveling ? FString::Printf(TEXT("I could use company. Heading to %s."), *VenueName) : TEXT("It's good to connect with people.");
		break;
	case EUEGT1SimActionType::Work:
		{
			const FString Role = Venue && !Venue->JobTitle.IsEmpty() ? Venue->JobTitle : FString(TEXT("staff"));
		NPC.CurrentThought = bTraveling
			? FString::Printf(TEXT("I want an hour as %s at %s — $%.0f/hr."), *Role, *VenueName, Venue ? Venue->HourlyRate : 0.0f)
			: FString::Printf(TEXT("Working as %s at %s, $%.0f/hr."), *Role, *VenueName, Venue ? Venue->HourlyRate : 0.0f);
		}
		break;
	case EUEGT1SimActionType::Idle:
	default:
		if (NPC.CurrentWorkSessionMinutes > 0.0f && Venue && Venue->VenueType == EUEGT1TownVenueType::Workplace)
		{
			NPC.CurrentThought = FString::Printf(TEXT("That hour paid $%.0f. Do I want another?"), Venue->HourlyRate);
		}
		else if (NPC.Needs.Hunger < NPC.Needs.Energy && NPC.Needs.Hunger < NPC.Needs.Hygiene && NPC.Needs.Hunger < NPC.Needs.Social)
		{
			NPC.CurrentThought = TEXT("Food is on my mind.");
		}
		else if (NPC.Needs.Energy < NPC.Needs.Hygiene && NPC.Needs.Energy < NPC.Needs.Social)
		{
			NPC.CurrentThought = TEXT("I need to rest soon.");
		}
		else if (NPC.Money < Tuning.BrokeThreshold)
		{
			NPC.CurrentThought = FString::Printf(TEXT("I need money. My goal is $%.0f."), NPC.SavingsGoal);
		}
		else
		{
			NPC.CurrentThought = TEXT("Taking a moment to decide what's next.");
		}
		break;
	}
}

void FUEGT1TownSimulationModel::Reserve(FUEGT1TownVenueState& Venue, FName NpcId)
{
	Venue.Reservations.AddUnique(NpcId);
}

void FUEGT1TownSimulationModel::ReleaseReservation(FName VenueId, FName NpcId)
{
	if (FUEGT1TownVenueState* Venue = FindVenue(VenueId))
	{
		Venue->Reservations.Remove(NpcId);
	}
}

FUEGT1NPCSimulationState* FUEGT1TownSimulationModel::FindNPC(FName NpcId)
{
	return NPCs.FindByPredicate([NpcId](const FUEGT1NPCSimulationState& NPC) { return NPC.NpcId == NpcId; });
}

FUEGT1TownVenueState* FUEGT1TownSimulationModel::FindVenue(FName VenueId)
{
	return Venues.FindByPredicate([VenueId](const FUEGT1TownVenueState& Venue) { return Venue.VenueId == VenueId; });
}

const FUEGT1TownVenueState* FUEGT1TownSimulationModel::FindVenue(FName VenueId) const
{
	return Venues.FindByPredicate([VenueId](const FUEGT1TownVenueState& Venue) { return Venue.VenueId == VenueId; });
}

const FUEGT1ActionDefinition* FUEGT1TownSimulationModel::FindActionDefinition(EUEGT1SimActionType Action,
	EUEGT1TownVenueType VenueType) const
{
	return Tuning.ActionDefinitions.FindByPredicate([Action, VenueType](const FUEGT1ActionDefinition& Definition)
	{
		return Definition.Action == Action && Definition.VenueType == VenueType;
	});
}

float FUEGT1TownSimulationModel::CalculateTravelDistance(const FUEGT1NPCSimulationState& NPC,
	const FUEGT1TownVenueState& Destination, EUEGT1SimActionType Action) const
{
	const TArray<FVector> Path = BuildTravelPath(NPC, Destination, Action);
	float Distance = 0.0f;
	for (int32 Index = 1; Index < Path.Num(); ++Index)
	{
		Distance += FVector::Dist2D(Path[Index - 1], Path[Index]);
	}
	return Distance;
}

TArray<FVector> FUEGT1TownSimulationModel::BuildTravelPath(const FUEGT1NPCSimulationState& NPC,
	const FUEGT1TownVenueState& Destination, EUEGT1SimActionType Action) const
{
	const FUEGT1TownVenueState* CurrentVenue = FindVenue(NPC.CurrentVenueId);
	return UEGT1TownGeneration::BuildVenuePath(NPC.WorldLocation, CurrentVenue, Destination,
		GetActivityLocation(NPC, Destination, Action));
}

FVector FUEGT1TownSimulationModel::GetActivityLocation(const FUEGT1NPCSimulationState& NPC,
	const FUEGT1TownVenueState& Destination, EUEGT1SimActionType Action) const
{
	if (Action == EUEGT1SimActionType::Sleep && Destination.VenueType == EUEGT1TownVenueType::Home)
	{
		if (const FUEGT1TownBedState* Bed = Destination.Beds.FindByPredicate([&NPC](const FUEGT1TownBedState& Candidate)
		{
			return Candidate.BedId == NPC.BedId;
		}))
		{
			return Bed->WorldLocation;
		}
	}
	if (Action == EUEGT1SimActionType::Eat && Destination.VenueType == EUEGT1TownVenueType::Home)
	{
		return Destination.KitchenLocation;
	}
	if (Action == EUEGT1SimActionType::Hygiene)
	{
		return Destination.ShowerLocation;
	}
	return Destination.ActivityLocation;
}

bool FUEGT1TownSimulationModel::CanPlayerPerformActivity(EUEGT1SimActionType Action, FName VenueId, FString& OutReason) const
{
	const FUEGT1TownVenueState* Venue = FindVenue(VenueId);
	const FUEGT1ActionDefinition* Definition = Venue ? FindActionDefinition(Action, Venue->VenueType) : nullptr;
	if (!Venue || !Definition || Action == EUEGT1SimActionType::Idle || Action == EUEGT1SimActionType::Travel)
	{
		OutReason = TEXT("That activity is not available here.");
		return false;
	}
	const float Duration = Action == EUEGT1SimActionType::Work ? Tuning.WorkDecisionIntervalMinutes : Definition->DurationMinutes;
	if (!Venue->IsOpenForDuration(GetHourOfDay(), Duration))
	{
		OutReason = FString::Printf(TEXT("%s is closed or closes too soon."), *Venue->DisplayName);
		return false;
	}
	if (Player.Money + KINDA_SMALL_NUMBER < Definition->Cost)
	{
		OutReason = FString::Printf(TEXT("You need $%.0f but have $%.0f."), Definition->Cost, Player.Money);
		return false;
	}
	OutReason.Reset();
	return true;
}

bool FUEGT1TownSimulationModel::PerformPlayerActivity(EUEGT1SimActionType Action, FName VenueId, FString& OutResult)
{
	if (!CanPlayerPerformActivity(Action, VenueId, OutResult))
	{
		Player.LastActivityMessage = OutResult;
		return false;
	}
	const FUEGT1TownVenueState* Venue = FindVenue(VenueId);
	const FUEGT1ActionDefinition* Definition = Venue ? FindActionDefinition(Action, Venue->VenueType) : nullptr;
	if (!Venue || !Definition)
	{
		OutResult = TEXT("The activity definition disappeared.");
		return false;
	}
	const float Duration = Action == EUEGT1SimActionType::Work ? Tuning.WorkDecisionIntervalMinutes : Definition->DurationMinutes;
	const float Cost = Definition->Cost;
	const float Earnings = Action == EUEGT1SimActionType::Work ? Venue->HourlyRate * Duration / 60.0f : Definition->Earnings;
	const FUEGT1NeedState Effects = Definition->NeedEffects;
	AdvanceMinutes(Duration);
	Player.Money = FMath::Max(0.0f, Player.Money - Cost + Earnings);
	Player.Needs.Energy += Effects.Energy;
	Player.Needs.Hunger += Effects.Hunger;
	Player.Needs.Hygiene += Effects.Hygiene;
	Player.Needs.Social += Effects.Social;
	Player.Needs.Clamp();
	Player.LastAction = Action;
	Player.LastVenueId = VenueId;
	++Player.CompletedActivities;
	Metrics.MoneySpent += Cost;
	Metrics.MoneyEarned += Earnings;
	if (Action == EUEGT1SimActionType::Work)
	{
		OutResult = FString::Printf(TEXT("Worked one hour as %s at %s and earned $%.0f."),
			Venue->JobTitle.IsEmpty() ? TEXT("staff") : *Venue->JobTitle, *Venue->DisplayName, Earnings);
	}
	else
	{
		OutResult = FString::Printf(TEXT("Completed %s at %s."), LexToString(Action), *Venue->DisplayName);
	}
	Player.LastActivityMessage = OutResult;
	return true;
}

void UEGT1TownSimulation::ApplyActionOutcome(FUEGT1NPCSimulationState& NPC, const FUEGT1ActionDefinition& Definition,
	FUEGT1SimulationMetrics& Metrics)
{
	NPC.Money = FMath::Max(0.0f, NPC.Money - Definition.Cost + Definition.Earnings);
	Metrics.MoneySpent += Definition.Cost;
	Metrics.MoneyEarned += Definition.Earnings;
	NPC.Needs.Energy += Definition.NeedEffects.Energy;
	NPC.Needs.Hunger += Definition.NeedEffects.Hunger;
	NPC.Needs.Hygiene += Definition.NeedEffects.Hygiene;
	NPC.Needs.Social += Definition.NeedEffects.Social;
	NPC.Needs.Clamp();
	switch (Definition.Action)
	{
	case EUEGT1SimActionType::Sleep: ++NPC.CompletedSleepActions; break;
	case EUEGT1SimActionType::Eat: ++NPC.CompletedEatActions; break;
	case EUEGT1SimActionType::Hygiene: ++NPC.CompletedHygieneActions; break;
	case EUEGT1SimActionType::Socialize: ++NPC.CompletedSocialActions; break;
	case EUEGT1SimActionType::Work: ++NPC.CompletedWorkActions; break;
	default: break;
	}
}
