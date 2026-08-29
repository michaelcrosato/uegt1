#include "Simulation/UEGT1TownSimulationTypes.h"

const TCHAR* LexToString(EUEGT1TownVenueType VenueType)
{
	switch (VenueType)
	{
	case EUEGT1TownVenueType::Home: return TEXT("Home");
	case EUEGT1TownVenueType::FoodVenue: return TEXT("Food");
	case EUEGT1TownVenueType::Workplace: return TEXT("Workplace");
	case EUEGT1TownVenueType::SocialVenue: return TEXT("Social");
	case EUEGT1TownVenueType::Park: return TEXT("Park");
	default: return TEXT("Unknown");
	}
}

const TCHAR* LexToString(EUEGT1SimActionType ActionType)
{
	switch (ActionType)
	{
	case EUEGT1SimActionType::Idle: return TEXT("Idle");
	case EUEGT1SimActionType::Travel: return TEXT("Travel");
	case EUEGT1SimActionType::Sleep: return TEXT("Sleep");
	case EUEGT1SimActionType::Eat: return TEXT("Eat");
	case EUEGT1SimActionType::Hygiene: return TEXT("Hygiene");
	case EUEGT1SimActionType::Socialize: return TEXT("Socialize");
	case EUEGT1SimActionType::Work: return TEXT("Work");
	default: return TEXT("Unknown");
	}
}

const TCHAR* LexToString(EUEGT1HouseholdRelationship Relationship)
{
	switch (Relationship)
	{
	case EUEGT1HouseholdRelationship::Family: return TEXT("Family");
	case EUEGT1HouseholdRelationship::Friend: return TEXT("Friend");
	case EUEGT1HouseholdRelationship::Roommate: return TEXT("Roommate");
	default: return TEXT("Unknown");
	}
}

void FUEGT1NeedState::Clamp()
{
	Energy = FMath::Clamp(Energy, 0.0f, 1.0f);
	Hunger = FMath::Clamp(Hunger, 0.0f, 1.0f);
	Hygiene = FMath::Clamp(Hygiene, 0.0f, 1.0f);
	Social = FMath::Clamp(Social, 0.0f, 1.0f);
}

bool FUEGT1DailySchedule::IsWorkTime(int32 DayIndex, float Hour) const
{
	const int32 DayOfWeek = FMath::Abs(DayIndex) % 7;
	if ((WorkdayMask & (1u << DayOfWeek)) == 0)
	{
		return false;
	}
	if (FMath::IsNearlyEqual(WorkStartHour, WorkEndHour))
	{
		return true;
	}
	return WorkStartHour <= WorkEndHour
		? Hour >= WorkStartHour && Hour < WorkEndHour
		: Hour >= WorkStartHour || Hour < WorkEndHour;
}

bool FUEGT1DailySchedule::IsSleepTime(float Hour) const
{
	return SleepStartHour <= SleepEndHour
		? Hour >= SleepStartHour && Hour < SleepEndHour
		: Hour >= SleepStartHour || Hour < SleepEndHour;
}

bool FUEGT1TownVenueState::IsOpen(float Hour) const
{
	if (FMath::IsNearlyEqual(OpeningHour, ClosingHour) || (OpeningHour <= 0.0f && ClosingHour >= 24.0f))
	{
		return true;
	}
	return OpeningHour < ClosingHour
		? Hour >= OpeningHour && Hour < ClosingHour
		: Hour >= OpeningHour || Hour < ClosingHour;
}

bool FUEGT1TownVenueState::IsOpenForDuration(float StartHour, float DurationMinutes) const
{
	if (!IsOpen(StartHour))
	{
		return false;
	}
	if (FMath::IsNearlyEqual(OpeningHour, ClosingHour) || (OpeningHour <= 0.0f && ClosingHour >= 24.0f))
	{
		return true;
	}

	const float NormalizedStart = FMath::Fmod(FMath::Fmod(StartHour, 24.0f) + 24.0f, 24.0f);
	float HoursUntilClose = 0.0f;
	if (OpeningHour < ClosingHour)
	{
		HoursUntilClose = ClosingHour - NormalizedStart;
	}
	else
	{
		HoursUntilClose = NormalizedStart >= OpeningHour
			? (24.0f - NormalizedStart) + ClosingHour
			: ClosingHour - NormalizedStart;
	}
	return DurationMinutes / 60.0f <= HoursUntilClose + KINDA_SMALL_NUMBER;
}

bool FUEGT1TownVenueState::HasCapacityFor(FName NpcId) const
{
	return Reservations.Contains(NpcId) || Reservations.Num() < FMath::Max(Capacity, 0);
}
