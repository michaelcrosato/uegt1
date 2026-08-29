#include "World/UEGT1TownGeneration.h"

#include "World/UEGT1WorldLayout.h"

namespace
{
	constexpr float StreetMinX = -22500.0f;
	constexpr float StreetMaxX = 4700.0f;
	constexpr float StreetHalfExtentY = 5600.0f;
	constexpr float StreetXCoordinates[] = {
		-21600.0f, -19800.0f, -18000.0f, -16200.0f, -14400.0f, -12600.0f, -10800.0f, -9000.0f, -7200.0f,
		-5400.0f, -3600.0f, -1800.0f, 0.0f, 1800.0f
	};
	constexpr float StreetYCoordinates[] = { -3600.0f, -1200.0f, 1200.0f, 3600.0f };
	constexpr float LotYCoordinates[] = { -4750.0f, -2400.0f, 0.0f, 2400.0f, 4750.0f };

	template <uint32 CoordinateCount>
	float NearestStreetCoordinate(float Value, const float (&Coordinates)[CoordinateCount])
	{
		float Best = Coordinates[0];
		float BestDistance = FMath::Abs(Value - Best);
		for (const float Coordinate : Coordinates)
		{
			const float Distance = FMath::Abs(Value - Coordinate);
			if (Distance < BestDistance)
			{
				Best = Coordinate;
				BestDistance = Distance;
			}
		}
		return Best;
	}

	FVector MakeAccessPoint(const FVector& LotCenter)
	{
		const float StreetX = NearestStreetCoordinate(LotCenter.X, StreetXCoordinates);
		const float StreetY = NearestStreetCoordinate(LotCenter.Y, StreetYCoordinates);
		if (FMath::Abs(LotCenter.X - StreetX) < FMath::Abs(LotCenter.Y - StreetY))
		{
			return FVector(StreetX, LotCenter.Y, 12.0f);
		}
		return FVector(LotCenter.X, StreetY, 12.0f);
	}

	FVector NearestIntersection(const FVector& Point)
	{
		return FVector(NearestStreetCoordinate(Point.X, StreetXCoordinates),
			NearestStreetCoordinate(Point.Y, StreetYCoordinates), Point.Z);
	}

	void ConfigureVenue(FUEGT1GeneratedTownLot& Lot, const FUEGT1SimulationTuning& Tuning, int32 TypeIndex)
	{
		Lot.DisplayName = Lot.LotId.ToString();
		switch (Lot.VenueType)
		{
		case EUEGT1TownVenueType::Home:
			Lot.Capacity = Tuning.HomeCapacity;
			Lot.DisplayName = FString::Printf(TEXT("Residence %02d"), TypeIndex);
			Lot.BusinessType = TEXT("Residence");
			break;
		case EUEGT1TownVenueType::FoodVenue:
			Lot.Capacity = Tuning.FoodVenueCapacity;
			Lot.OpeningHour = Tuning.FoodVenueOpenHour;
			Lot.ClosingHour = Tuning.FoodVenueCloseHour;
			Lot.DisplayName = FString::Printf(TEXT("Town Restaurant %02d"), TypeIndex);
			Lot.BusinessType = TEXT("Restaurant");
			break;
		case EUEGT1TownVenueType::Workplace:
			if (Tuning.JobDefinitions.IsValidIndex(TypeIndex - 1))
			{
				const FUEGT1JobDefinition& Job = Tuning.JobDefinitions[TypeIndex - 1];
				Lot.LotId = Job.JobId;
				Lot.DisplayName = Job.BusinessName.IsEmpty() ? Job.DisplayName : Job.BusinessName;
				Lot.JobTitle = Job.DisplayName;
				Lot.BusinessType = Job.BusinessType;
				Lot.Capacity = Job.Capacity;
				Lot.OpeningHour = Job.OpeningHour;
				Lot.ClosingHour = Job.ClosingHour;
				Lot.HourlyRate = Job.HourlyRate;
			}
			else
			{
				Lot.Capacity = Tuning.WorkplaceCapacity;
				Lot.OpeningHour = Tuning.WorkplaceOpenHour;
				Lot.ClosingHour = Tuning.WorkplaceCloseHour;
				Lot.HourlyRate = 18.0f;
			}
			break;
		case EUEGT1TownVenueType::SocialVenue:
			Lot.Capacity = Tuning.SocialVenueCapacity;
			Lot.OpeningHour = Tuning.SocialVenueOpenHour;
			Lot.ClosingHour = Tuning.SocialVenueCloseHour;
			Lot.DisplayName = FString::Printf(TEXT("Community Hall %02d"), TypeIndex);
			Lot.BusinessType = TEXT("Community Venue");
			break;
		case EUEGT1TownVenueType::Park:
			Lot.Capacity = Tuning.ParkCapacity;
			Lot.DisplayName = FString::Printf(TEXT("Town Park %02d"), TypeIndex);
			Lot.BusinessType = TEXT("Park");
			break;
		default:
			break;
		}
	}

	void ConfigureInterior(FUEGT1GeneratedTownLot& Lot)
	{
		const float GroundHeight = UEGT1WorldLayout::SampleRegion(Lot.Center).SurfaceHeight;
		if (Lot.VenueType == EUEGT1TownVenueType::Park)
		{
			Lot.FacingRotation = FRotator::ZeroRotator;
			Lot.EntranceLocation = Lot.AccessPoint;
			Lot.InteriorEntryLocation = Lot.Center;
			Lot.KitchenLocation = Lot.Center;
			Lot.BathroomLocation = Lot.Center;
			Lot.ShowerLocation = Lot.Center;
			Lot.ActivityLocation = Lot.Center;
			Lot.ActivityLocation.Z = GroundHeight;
			return;
		}

		Lot.FacingRotation = (Lot.AccessPoint - Lot.Center).Rotation();
		Lot.FacingRotation.Pitch = 0.0f;
		Lot.FacingRotation.Roll = 0.0f;
		const FVector Forward = Lot.FacingRotation.Vector();
		const FVector Right = FRotationMatrix(Lot.FacingRotation).GetScaledAxis(EAxis::Y);
		const float HalfWidth = Lot.Footprint.X * 0.5f;
		const float HalfDepth = Lot.Footprint.Y * 0.5f;
		Lot.EntranceLocation = Lot.Center + Forward * (HalfWidth + 28.0f);
		Lot.InteriorEntryLocation = Lot.Center + Forward * (HalfWidth - 125.0f);
		Lot.KitchenLocation = Lot.Center + Forward * (Lot.Footprint.X * 0.12f) - Right * (Lot.Footprint.Y * 0.27f);
		Lot.BathroomLocation = Lot.Center - Forward * (Lot.Footprint.X * 0.22f) + Right * (Lot.Footprint.Y * 0.28f);
		Lot.ShowerLocation = Lot.BathroomLocation - Forward * 55.0f;
		Lot.ActivityLocation = Lot.Center - Forward * (Lot.Footprint.X * 0.02f);
		for (FVector* Point : { &Lot.EntranceLocation, &Lot.InteriorEntryLocation, &Lot.KitchenLocation,
			&Lot.BathroomLocation, &Lot.ShowerLocation, &Lot.ActivityLocation })
		{
			Point->Z = GroundHeight;
		}
		if (Lot.VenueType != EUEGT1TownVenueType::Home)
		{
			FVector AmenityBed = Lot.Center - Forward * (Lot.Footprint.X * 0.25f) - Right * (Lot.Footprint.Y * 0.24f);
			AmenityBed.Z = GroundHeight;
			Lot.AmenityBedLocations.Add(AmenityBed);
		}
	}

	void AddBeds(FUEGT1GeneratedTownLot& Lot)
	{
		if (Lot.VenueType != EUEGT1TownVenueType::Home)
		{
			return;
		}

		const FVector Front = Lot.FacingRotation.Vector();
		const FVector Right = FRotationMatrix(Lot.FacingRotation).GetScaledAxis(EAxis::Y);
		const float GroundHeight = UEGT1WorldLayout::SampleRegion(Lot.Center).SurfaceHeight;
		for (int32 SlotIndex = 0; SlotIndex < Lot.Capacity; ++SlotIndex)
		{
			const int32 Row = SlotIndex / 2;
			const int32 Column = SlotIndex % 2;
			FUEGT1TownBedState Bed;
			Bed.BedId = FName(*FString::Printf(TEXT("%s_Bed_%02d"), *Lot.LotId.ToString(), SlotIndex + 1));
			Bed.WorldLocation = Lot.Center + Front * ((Row == 0 ? -1.0f : 1.0f) * Lot.Footprint.X * 0.18f) +
				Right * ((Column == 0 ? -1.0f : 1.0f) * Lot.Footprint.Y * 0.28f);
			Bed.WorldLocation.Z = GroundHeight;
			Bed.Rotation = Lot.FacingRotation;
			Lot.Beds.Add(MoveTemp(Bed));
		}
	}
}

FUEGT1GeneratedTownLayout UEGT1TownGeneration::Generate(int32 Seed, const FUEGT1SimulationTuning& Tuning)
{
	FUEGT1GeneratedTownLayout Layout;
	Layout.Seed = Seed;
	Layout.BoundsMin = FVector2D(StreetMinX, -StreetHalfExtentY);
	Layout.BoundsMax = FVector2D(StreetMaxX, StreetHalfExtentY);
	for (const float Coordinate : StreetYCoordinates)
	{
		FUEGT1GeneratedStreet Horizontal;
		Horizontal.Start = FVector(StreetMinX, Coordinate, 4.0f);
		Horizontal.End = FVector(StreetMaxX, Coordinate, 4.0f);
		Layout.Streets.Add(Horizontal);
	}
	for (const float Coordinate : StreetXCoordinates)
	{
		FUEGT1GeneratedStreet Vertical;
		Vertical.Start = FVector(Coordinate, -StreetHalfExtentY, 4.0f);
		Vertical.End = FVector(Coordinate, StreetHalfExtentY, 4.0f);
		Layout.Streets.Add(Vertical);
	}
	for (const float X : StreetXCoordinates)
	{
		for (const float Y : StreetYCoordinates)
		{
			Layout.Intersections.Add(FVector(X, Y, 8.0f));
		}
	}

	TArray<FVector> LotCenters;
	for (int32 XIndex = 0; XIndex + 1 < static_cast<int32>(UE_ARRAY_COUNT(StreetXCoordinates)); ++XIndex)
	{
		const float LotX = (StreetXCoordinates[XIndex] + StreetXCoordinates[XIndex + 1]) * 0.5f;
		for (const float LotY : LotYCoordinates)
		{
			const FVector Candidate(LotX, LotY, 0.0f);
			if (FVector::Dist2D(Candidate, FVector::ZeroVector) >= 1500.0f &&
				!UEGT1WorldLayout::IsPrimaryRoute(Candidate, 760.0f))
			{
				LotCenters.Add(Candidate);
			}
		}
	}
	TArray<EUEGT1TownVenueType> VenueTypes = {
		EUEGT1TownVenueType::Home, EUEGT1TownVenueType::Home, EUEGT1TownVenueType::Home, EUEGT1TownVenueType::Home,
		EUEGT1TownVenueType::Home, EUEGT1TownVenueType::Home, EUEGT1TownVenueType::Home, EUEGT1TownVenueType::Home,
		EUEGT1TownVenueType::Home, EUEGT1TownVenueType::Home, EUEGT1TownVenueType::Home, EUEGT1TownVenueType::Home,
		EUEGT1TownVenueType::Home, EUEGT1TownVenueType::Home, EUEGT1TownVenueType::Home, EUEGT1TownVenueType::Home,
		EUEGT1TownVenueType::Home, EUEGT1TownVenueType::Home, EUEGT1TownVenueType::Home, EUEGT1TownVenueType::Home,
		EUEGT1TownVenueType::Home, EUEGT1TownVenueType::Home, EUEGT1TownVenueType::Home, EUEGT1TownVenueType::Home,
		EUEGT1TownVenueType::Home,
		EUEGT1TownVenueType::FoodVenue, EUEGT1TownVenueType::FoodVenue,
		EUEGT1TownVenueType::Workplace, EUEGT1TownVenueType::Workplace, EUEGT1TownVenueType::Workplace, EUEGT1TownVenueType::Workplace,
		EUEGT1TownVenueType::Workplace, EUEGT1TownVenueType::Workplace, EUEGT1TownVenueType::Workplace, EUEGT1TownVenueType::Workplace,
		EUEGT1TownVenueType::Workplace, EUEGT1TownVenueType::Workplace, EUEGT1TownVenueType::Workplace, EUEGT1TownVenueType::Workplace,
		EUEGT1TownVenueType::Workplace, EUEGT1TownVenueType::Workplace, EUEGT1TownVenueType::Workplace, EUEGT1TownVenueType::Workplace,
		EUEGT1TownVenueType::Workplace, EUEGT1TownVenueType::Workplace, EUEGT1TownVenueType::Workplace, EUEGT1TownVenueType::Workplace,
		EUEGT1TownVenueType::SocialVenue, EUEGT1TownVenueType::SocialVenue,
		EUEGT1TownVenueType::Park, EUEGT1TownVenueType::Park, EUEGT1TownVenueType::Park
	};

	FRandomStream Random(Seed + 401);
	for (int32 Index = LotCenters.Num() - 1; Index > 0; --Index)
	{
		LotCenters.Swap(Index, Random.RandRange(0, Index));
	}
	for (int32 Index = VenueTypes.Num() - 1; Index > 0; --Index)
	{
		const int32 Other = Random.RandRange(0, Index);
		VenueTypes.Swap(Index, Other);
	}
	checkf(LotCenters.Num() >= VenueTypes.Num(), TEXT("The generated town footprint must provide at least %d route-safe lots."), VenueTypes.Num());

	TMap<EUEGT1TownVenueType, int32> TypeCounts;
	for (int32 Index = 0; Index < VenueTypes.Num(); ++Index)
	{
		FUEGT1GeneratedTownLot Lot;
		Lot.Center = LotCenters[Index];
		Lot.AccessPoint = MakeAccessPoint(Lot.Center);
		Lot.VenueType = VenueTypes[Index];
		Lot.Footprint = FVector2D(Random.FRandRange(780.0f, 1120.0f), Random.FRandRange(680.0f, 940.0f));
		Lot.BuildingHeight = Lot.VenueType == EUEGT1TownVenueType::Workplace
			? Random.FRandRange(900.0f, 1250.0f) : Random.FRandRange(580.0f, 920.0f);
		const int32 TypeIndex = TypeCounts.FindOrAdd(Lot.VenueType) + 1;
		TypeCounts[Lot.VenueType] = TypeIndex;
		Lot.LotId = FName(*FString::Printf(TEXT("%s_%02d"), LexToString(Lot.VenueType), TypeIndex));
		ConfigureVenue(Lot, Tuning, TypeIndex);
		ConfigureInterior(Lot);
		AddBeds(Lot);
		Layout.Lots.Add(MoveTemp(Lot));
	}
	return Layout;
}

bool UEGT1TownGeneration::IsFullyConnected(const FUEGT1GeneratedTownLayout& Layout)
{
	if (Layout.Streets.Num() < 2 || Layout.Intersections.IsEmpty() || Layout.Lots.IsEmpty())
	{
		return false;
	}
	for (const FUEGT1GeneratedTownLot& Lot : Layout.Lots)
	{
		bool bTouchesStreet = false;
		for (const FUEGT1GeneratedStreet& Street : Layout.Streets)
		{
			const FVector Closest = FMath::ClosestPointOnSegment(Lot.AccessPoint, Street.Start, Street.End);
			if (FVector::Dist2D(Closest, Lot.AccessPoint) < 1.0f)
			{
				bTouchesStreet = true;
				break;
			}
		}
		if (!bTouchesStreet)
		{
			return false;
		}
	}
	return true;
}

TArray<FVector> UEGT1TownGeneration::BuildStreetPath(const FVector& Start, const FVector& StartAccess,
	const FVector& DestinationAccess, const FVector& Destination)
{
	TArray<FVector> Path;
	Path.Add(Start);
	Path.Add(StartAccess);
	const FVector StartIntersection = NearestIntersection(StartAccess);
	const FVector DestinationIntersection = NearestIntersection(DestinationAccess);
	Path.Add(StartIntersection);
	Path.Add(FVector(DestinationIntersection.X, StartIntersection.Y, StartIntersection.Z));
	Path.Add(DestinationIntersection);
	Path.Add(DestinationAccess);
	Path.Add(Destination);

	for (int32 Index = Path.Num() - 1; Index > 0; --Index)
	{
		if (FVector::DistSquared(Path[Index], Path[Index - 1]) < 1.0f)
		{
			Path.RemoveAt(Index);
		}
	}
	return Path;
}

TArray<FVector> UEGT1TownGeneration::BuildVenuePath(const FVector& Start, const FUEGT1TownVenueState* CurrentVenue,
	const FUEGT1TownVenueState& DestinationVenue, const FVector& ActivityLocation)
{
	TArray<FVector> Path;
	auto AddUniquePoint = [&Path](const FVector& Point)
	{
		if (Path.IsEmpty() || FVector::DistSquared(Path.Last(), Point) >= 1.0f)
		{
			Path.Add(Point);
		}
	};

	AddUniquePoint(Start);
	if (CurrentVenue && CurrentVenue->VenueId == DestinationVenue.VenueId)
	{
		AddUniquePoint(DestinationVenue.InteriorEntryLocation);
		AddUniquePoint(ActivityLocation);
		return Path;
	}
	FVector StreetStart = Start;
	if (CurrentVenue && CurrentVenue->VenueType != EUEGT1TownVenueType::Park)
	{
		AddUniquePoint(CurrentVenue->InteriorEntryLocation);
		AddUniquePoint(CurrentVenue->EntranceLocation);
		AddUniquePoint(CurrentVenue->AccessPoint);
		StreetStart = CurrentVenue->AccessPoint;
	}
	const TArray<FVector> StreetPath = BuildStreetPath(StreetStart, StreetStart,
		DestinationVenue.AccessPoint, DestinationVenue.AccessPoint);
	for (const FVector& Point : StreetPath)
	{
		AddUniquePoint(Point);
	}
	if (DestinationVenue.VenueType != EUEGT1TownVenueType::Park)
	{
		AddUniquePoint(DestinationVenue.EntranceLocation);
		AddUniquePoint(DestinationVenue.InteriorEntryLocation);
	}
	AddUniquePoint(ActivityLocation);
	return Path;
}
