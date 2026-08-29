#include "Simulation/UEGT1TownSimulationSettings.h"

namespace
{
	FUEGT1ActionDefinition MakeAction(EUEGT1SimActionType Action, EUEGT1TownVenueType Venue, float Duration,
		float Cost, float Earnings, const FUEGT1NeedState& Effects, float BaseUtility, float NeedWeight,
		bool bPreferAssigned = false, bool bRequiresSchedule = false)
	{
		FUEGT1ActionDefinition Definition;
		Definition.Action = Action;
		Definition.VenueType = Venue;
		Definition.DurationMinutes = Duration;
		Definition.Cost = Cost;
		Definition.Earnings = Earnings;
		Definition.NeedEffects = Effects;
		Definition.BaseUtility = BaseUtility;
		Definition.NeedWeight = NeedWeight;
		Definition.bPreferAssignedDestination = bPreferAssigned;
		Definition.bRequiresWorkSchedule = bRequiresSchedule;
		return Definition;
	}

	FUEGT1JobDefinition MakeJob(const TCHAR* JobId, const TCHAR* DisplayName, const TCHAR* BusinessName,
		const TCHAR* BusinessType, float OpeningHour, float ClosingHour, float HourlyRate, int32 Capacity = 5)
	{
		FUEGT1JobDefinition Definition;
		Definition.JobId = FName(JobId);
		Definition.DisplayName = DisplayName;
		Definition.BusinessName = BusinessName;
		Definition.BusinessType = BusinessType;
		Definition.OpeningHour = OpeningHour;
		Definition.ClosingHour = ClosingHour;
		Definition.HourlyRate = HourlyRate;
		Definition.Capacity = Capacity;
		return Definition;
	}
}

UUEGT1TownSimulationSettings::UUEGT1TownSimulationSettings()
{
	ActionDefinitions = {
		MakeAction(EUEGT1SimActionType::Sleep, EUEGT1TownVenueType::Home, 360.0f, 0.0f, 0.0f,
			FUEGT1NeedState{ 0.92f, -0.14f, -0.05f, -0.04f }, 4.0f, 130.0f, true),
		MakeAction(EUEGT1SimActionType::Eat, EUEGT1TownVenueType::FoodVenue, 45.0f, 12.0f, 0.0f,
			FUEGT1NeedState{ 0.02f, 0.78f, 0.0f, 0.05f }, 7.0f, 115.0f),
		MakeAction(EUEGT1SimActionType::Eat, EUEGT1TownVenueType::Home, 55.0f, 1.0f, 0.0f,
			FUEGT1NeedState{ 0.0f, 0.46f, -0.02f, 0.0f }, -4.0f, 92.0f, true),
		MakeAction(EUEGT1SimActionType::Hygiene, EUEGT1TownVenueType::Home, 50.0f, 0.0f, 0.0f,
			FUEGT1NeedState{ 0.0f, -0.02f, 0.78f, 0.0f }, 1.0f, 108.0f, true),
		MakeAction(EUEGT1SimActionType::Socialize, EUEGT1TownVenueType::SocialVenue, 90.0f, 8.0f, 0.0f,
			FUEGT1NeedState{ -0.04f, -0.05f, -0.02f, 0.70f }, 5.0f, 98.0f),
		MakeAction(EUEGT1SimActionType::Socialize, EUEGT1TownVenueType::Park, 100.0f, 0.0f, 0.0f,
			FUEGT1NeedState{ -0.03f, -0.04f, -0.01f, 0.42f }, -2.0f, 82.0f),
		MakeAction(EUEGT1SimActionType::Work, EUEGT1TownVenueType::Workplace, 60.0f, 0.0f, 0.0f,
			FUEGT1NeedState{ -0.04f, -0.03f, -0.02f, 0.01f }, 0.0f, 0.0f, true, false)
	};
	JobDefinitions = {
		MakeJob(TEXT("Baker"), TEXT("Baker"), TEXT("Sunrise Bakery"), TEXT("Bakery"), 6.0f, 14.0f, 17.0f),
		MakeJob(TEXT("Barista"), TEXT("Barista"), TEXT("Grove Cafe"), TEXT("Cafe"), 6.0f, 14.0f, 16.0f),
		MakeJob(TEXT("Dockworker"), TEXT("Dockworker"), TEXT("Signal Harbor Freight"), TEXT("Logistics"), 6.0f, 14.0f, 21.0f),
		MakeJob(TEXT("Groundskeeper"), TEXT("Groundskeeper"), TEXT("Parks Department"), TEXT("Public Service"), 6.0f, 14.0f, 16.0f),
		MakeJob(TEXT("Carpenter"), TEXT("Carpenter"), TEXT("Westwood Workshop"), TEXT("Factory & Construction"), 7.0f, 15.0f, 24.0f),
		MakeJob(TEXT("Mechanic"), TEXT("Mechanic"), TEXT("Waystone Auto Works"), TEXT("Repair Shop"), 7.0f, 15.0f, 23.0f),
		MakeJob(TEXT("Teacher"), TEXT("Teacher"), TEXT("Signal Grove School"), TEXT("Education"), 8.0f, 16.0f, 25.0f),
		MakeJob(TEXT("OfficeClerk"), TEXT("Office Clerk"), TEXT("Hearthstone Accounting"), TEXT("Professional Services"), 8.0f, 16.0f, 18.0f),
		MakeJob(TEXT("Courier"), TEXT("Courier"), TEXT("Grove Parcel Depot"), TEXT("Logistics"), 9.0f, 17.0f, 20.0f),
		MakeJob(TEXT("ClinicAssistant"), TEXT("Clinic Assistant"), TEXT("Signal Grove Clinic"), TEXT("Healthcare"), 9.0f, 17.0f, 22.0f),
		MakeJob(TEXT("RestaurantCook"), TEXT("Restaurant Cook"), TEXT("Lantern Restaurant"), TEXT("Restaurant"), 14.0f, 22.0f, 21.0f),
		MakeJob(TEXT("EveningServer"), TEXT("Evening Server"), TEXT("Moonrise Restaurant"), TEXT("Restaurant"), 14.0f, 22.0f, 19.0f),
		MakeJob(TEXT("Librarian"), TEXT("Librarian"), TEXT("Town Library"), TEXT("Civic Service"), 10.0f, 16.0f, 18.0f),
		MakeJob(TEXT("MuseumGuide"), TEXT("Museum Guide"), TEXT("Heritage Museum"), TEXT("Culture"), 10.0f, 16.0f, 18.0f),
		MakeJob(TEXT("MarketVendor"), TEXT("Market Vendor"), TEXT("Grove Grocery Market"), TEXT("Grocery Store"), 10.0f, 16.0f, 17.0f),
		MakeJob(TEXT("Florist"), TEXT("Florist"), TEXT("Wildflower Shop"), TEXT("Retail"), 10.0f, 16.0f, 19.0f),
		MakeJob(TEXT("NightWatch"), TEXT("Night Watch"), TEXT("SecureTown Services"), TEXT("Security"), 22.0f, 6.0f, 24.0f),
		MakeJob(TEXT("NightNurse"), TEXT("Night Nurse"), TEXT("Nightingale Hospital"), TEXT("Hospital"), 20.0f, 6.0f, 30.0f),
		MakeJob(TEXT("EmergencyDispatcher"), TEXT("Emergency Dispatcher"), TEXT("Emergency Services Center"), TEXT("Public Safety"), 0.0f, 0.0f, 28.0f),
		MakeJob(TEXT("TransitOperator"), TEXT("Transit Operator"), TEXT("Town Transit Depot"), TEXT("Transportation"), 0.0f, 0.0f, 23.0f)
	};
}

const UUEGT1TownSimulationSettings& UUEGT1TownSimulationSettings::Get()
{
	return *GetDefault<UUEGT1TownSimulationSettings>();
}

FUEGT1SimulationTuning UUEGT1TownSimulationSettings::MakeTuning() const
{
	FUEGT1SimulationTuning Result = Tuning;
	Result.ActionDefinitions = ActionDefinitions;
	Result.JobDefinitions = JobDefinitions;
	return Result;
}
