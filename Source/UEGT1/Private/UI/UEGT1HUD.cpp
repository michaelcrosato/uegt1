#include "UI/UEGT1HUD.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Gameplay/UEGT1MilestoneGameState.h"
#include "Interaction/UEGT1InteractionComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/UEGT1ExplorerCharacter.h"
#include "Player/UEGT1PlayerController.h"
#include "Simulation/UEGT1TownSimulationSubsystem.h"
#include "Simulation/UEGT1TownSimulationSettings.h"
#include "World/UEGT1Palette.h"
#include "World/UEGT1TownGeneration.h"
#include "World/UEGT1WorldLayout.h"

namespace
{
	constexpr int32 WorldMapColumns = 70;
	constexpr int32 WorldMapRows = 55;

	FLinearColor GetMapBiomeColor(const FUEGT1RegionSample& Region)
	{
		FLinearColor Color;
		switch (Region.GetDominantBiome())
		{
		case EUEGT1RegionBiome::Town: Color = FLinearColor(0.20f, 0.30f, 0.23f, 1.0f); break;
		case EUEGT1RegionBiome::Meadow: Color = FLinearColor(0.27f, 0.43f, 0.25f, 1.0f); break;
		case EUEGT1RegionBiome::Farmland: Color = FLinearColor(0.46f, 0.42f, 0.22f, 1.0f); break;
		case EUEGT1RegionBiome::Highlands: Color = FLinearColor(0.35f, 0.39f, 0.36f, 1.0f); break;
		case EUEGT1RegionBiome::Tropical: Color = FLinearColor(0.08f, 0.39f, 0.31f, 1.0f); break;
		case EUEGT1RegionBiome::Coast: Color = FLinearColor(0.66f, 0.57f, 0.35f, 1.0f); break;
		case EUEGT1RegionBiome::Ocean: Color = FLinearColor(0.025f, 0.18f, 0.29f, 1.0f); break;
		default: Color = FLinearColor(0.18f, 0.25f, 0.20f, 1.0f); break;
		}
		const float ElevationShade = FMath::Clamp(0.86f + Region.SurfaceHeight / 9000.0f, 0.72f, 1.10f);
		return Color * ElevationShade;
	}
}

void AUEGT1HUD::DrawHUD()
{
	Super::DrawHUD();
	if (!Canvas)
	{
		return;
	}
	if (const AUEGT1PlayerController* PlayerController = Cast<AUEGT1PlayerController>(GetOwningPlayerController());
		PlayerController && PlayerController->IsMenuOpen())
	{
		return;
	}

	const float ScreenWidth = Canvas->ClipX;
	const float ScreenHeight = Canvas->ClipY;
	if (const AUEGT1PlayerController* PlayerController = Cast<AUEGT1PlayerController>(GetOwningPlayerController());
		PlayerController && PlayerController->IsWorldMapOpen())
	{
		DrawWorldMap(ScreenWidth, ScreenHeight);
		return;
	}
	DrawObjectivePanel(ScreenWidth, ScreenHeight);
	DrawPlayerStatusPanel(ScreenWidth, ScreenHeight);
	DrawDeveloperModePanel(ScreenWidth, ScreenHeight);
	DrawInteractionPrompt(ScreenWidth, ScreenHeight);
	DrawResidentThoughtBubbles(ScreenWidth, ScreenHeight);
	DrawCrosshair(ScreenWidth, ScreenHeight);
	if (bShowDiagnostics)
	{
		DrawDiagnostics(ScreenWidth, ScreenHeight);
		DrawSimulationInspector(ScreenWidth, ScreenHeight);
	}
}

void AUEGT1HUD::EnsureWorldMapCache()
{
	const int32 WorldSeed = UEGT1WorldLayout::GetWorldSeed();
	if (CachedWorldMapSeed == WorldSeed && CachedWorldMapColors.Num() == WorldMapColumns * WorldMapRows)
	{
		return;
	}
	CachedWorldMapSeed = WorldSeed;
	CachedWorldMapColors.SetNumUninitialized(WorldMapColumns * WorldMapRows);
	const float MinX = UEGT1WorldLayout::GetWorldMinX();
	const float MaxX = UEGT1WorldLayout::GetWorldMaxX();
	const float MinY = UEGT1WorldLayout::GetWorldMinY();
	const float MaxY = UEGT1WorldLayout::GetWorldMaxY();
	for (int32 Row = 0; Row < WorldMapRows; ++Row)
	{
		for (int32 Column = 0; Column < WorldMapColumns; ++Column)
		{
			const FVector SampleLocation(
				FMath::Lerp(MinX, MaxX, (Column + 0.5f) / WorldMapColumns),
				FMath::Lerp(MaxY, MinY, (Row + 0.5f) / WorldMapRows), 0.0f);
			CachedWorldMapColors[Row * WorldMapColumns + Column] = GetMapBiomeColor(UEGT1WorldLayout::SampleRegion(SampleLocation));
		}
	}
}

void AUEGT1HUD::DrawWorldMap(float ScreenWidth, float ScreenHeight)
{
	EnsureWorldMapCache();
	DrawRect(FLinearColor(0.004f, 0.012f, 0.018f, 0.985f), 0.0f, 0.0f, ScreenWidth, ScreenHeight);
	DrawText(TEXT("SIGNAL GROVE ISLAND  /  WORLD MAP"), UEGT1Palette::Paper, 34.0f, 20.0f,
		GEngine->GetLargeFont(), 1.0f, false);
	DrawText(TEXT("M  CLOSE    •    YOU ARE HERE    •    ALL BUILDINGS AND KEY SERVICES"),
		UEGT1Palette::Signal, 36.0f, 56.0f, GEngine->GetSmallFont(), 0.82f, false);

	const float DirectoryWidth = FMath::Clamp(ScreenWidth * 0.285f, 390.0f, 540.0f);
	const float DirectoryX = ScreenWidth - DirectoryWidth - 28.0f;
	const float AvailableMapWidth = DirectoryX - 58.0f;
	const float AvailableMapHeight = ScreenHeight - 132.0f;
	const float WorldWidth = UEGT1WorldLayout::GetWorldMaxX() - UEGT1WorldLayout::GetWorldMinX();
	const float WorldHeight = UEGT1WorldLayout::GetWorldMaxY() - UEGT1WorldLayout::GetWorldMinY();
	float MapWidth = AvailableMapWidth;
	float MapHeight = MapWidth * WorldHeight / WorldWidth;
	if (MapHeight > AvailableMapHeight)
	{
		MapHeight = AvailableMapHeight;
		MapWidth = MapHeight * WorldWidth / WorldHeight;
	}
	const float MapX = 30.0f + (AvailableMapWidth - MapWidth) * 0.5f;
	const float MapY = 92.0f + (AvailableMapHeight - MapHeight) * 0.5f;
	DrawRect(FLinearColor(0.48f, 0.59f, 0.50f, 1.0f), MapX - 3.0f, MapY - 3.0f, MapWidth + 6.0f, MapHeight + 6.0f);

	const float CellWidth = MapWidth / WorldMapColumns;
	const float CellHeight = MapHeight / WorldMapRows;
	for (int32 Row = 0; Row < WorldMapRows; ++Row)
	{
		for (int32 Column = 0; Column < WorldMapColumns; ++Column)
		{
			DrawRect(CachedWorldMapColors[Row * WorldMapColumns + Column], MapX + Column * CellWidth,
				MapY + Row * CellHeight, CellWidth + 0.5f, CellHeight + 0.5f);
		}
	}

	const auto WorldToMap = [MapX, MapY, MapWidth, MapHeight](const FVector& Location)
	{
		const float AlphaX = (Location.X - UEGT1WorldLayout::GetWorldMinX()) /
			(UEGT1WorldLayout::GetWorldMaxX() - UEGT1WorldLayout::GetWorldMinX());
		const float AlphaY = (Location.Y - UEGT1WorldLayout::GetWorldMinY()) /
			(UEGT1WorldLayout::GetWorldMaxY() - UEGT1WorldLayout::GetWorldMinY());
		return FVector2D(MapX + FMath::Clamp(AlphaX, 0.0f, 1.0f) * MapWidth,
			MapY + (1.0f - FMath::Clamp(AlphaY, 0.0f, 1.0f)) * MapHeight);
	};

	DrawText(TEXT("HIGHLANDS"), FLinearColor(0.86f, 0.89f, 0.83f, 0.55f), MapX + MapWidth * 0.45f,
		MapY + 18.0f, GEngine->GetSmallFont(), 0.76f, false);
	DrawText(TEXT("WEST FARMS"), FLinearColor(0.94f, 0.86f, 0.58f, 0.62f), MapX + 18.0f,
		MapY + MapHeight * 0.49f, GEngine->GetSmallFont(), 0.72f, false);
	DrawText(TEXT("TROPICAL GROVE"), FLinearColor(0.64f, 0.92f, 0.75f, 0.58f), MapX + MapWidth * 0.40f,
		MapY + MapHeight - 30.0f, GEngine->GetSmallFont(), 0.72f, false);
	DrawText(TEXT("EASTERN COAST"), FLinearColor(0.76f, 0.88f, 0.94f, 0.60f), MapX + MapWidth - 150.0f,
		MapY + MapHeight * 0.49f, GEngine->GetSmallFont(), 0.70f, false);

	const UUEGT1TownSimulationSubsystem* Simulation = GetWorld()->GetSubsystem<UUEGT1TownSimulationSubsystem>();
	const FUEGT1TownSimulationModel* Model = Simulation && Simulation->IsSimulationRunning() ? &Simulation->GetModel() : nullptr;
	const int32 TownSeed = Model ? Model->GetSeed() : UUEGT1TownSimulationSettings::Get().TownSeed;
	const FUEGT1GeneratedTownLayout TownLayout = UEGT1TownGeneration::Generate(TownSeed,
		UUEGT1TownSimulationSettings::Get().MakeTuning());
	for (const FUEGT1GeneratedStreet& Street : TownLayout.Streets)
	{
		const FVector2D Start = WorldToMap(Street.Start);
		const FVector2D End = WorldToMap(Street.End);
		DrawLine(Start.X, Start.Y, End.X, End.Y, FLinearColor(0.80f, 0.74f, 0.58f, 0.70f), 2.0f);
	}

	const FVector2D Sanctuary = WorldToMap(UEGT1WorldLayout::GetSanctuaryLocation());
	const TArray<FVector>& WaystoneLocations = UEGT1WorldLayout::GetWaystoneLocations();
	const TArray<FName>& WaystoneIds = UEGT1WorldLayout::GetWaystoneIds();
	for (int32 Index = 0; Index < WaystoneLocations.Num(); ++Index)
	{
		const FVector2D Marker = WorldToMap(WaystoneLocations[Index]);
		DrawLine(Sanctuary.X, Sanctuary.Y, Marker.X, Marker.Y, FLinearColor(0.44f, 0.90f, 0.72f, 0.55f), 2.0f);
		DrawRect(UEGT1Palette::Amber, Marker.X - 5.0f, Marker.Y - 5.0f, 10.0f, 10.0f);
		DrawText(WaystoneIds[Index].ToString(), UEGT1Palette::Paper, Marker.X + 8.0f, Marker.Y - 7.0f,
			GEngine->GetSmallFont(), 0.62f, false);
	}
	DrawRect(UEGT1Palette::Signal, Sanctuary.X - 6.0f, Sanctuary.Y - 6.0f, 12.0f, 12.0f);
	DrawText(TEXT("SANCTUARY"), UEGT1Palette::Paper, Sanctuary.X + 9.0f, Sanctuary.Y - 8.0f,
		GEngine->GetSmallFont(), 0.68f, false);

	TArray<const FUEGT1TownVenueState*> KeyVenues;
	int32 HomeCount = 0;
	if (Model)
	{
		for (const FUEGT1TownVenueState& Venue : Model->GetVenues())
		{
			const FVector2D Marker = WorldToMap(Venue.WorldLocation);
			if (Venue.VenueType == EUEGT1TownVenueType::Home)
			{
				++HomeCount;
				DrawRect(FLinearColor(0.84f, 0.87f, 0.78f, 0.92f), Marker.X - 2.0f, Marker.Y - 2.0f, 4.0f, 4.0f);
			}
			else
			{
				KeyVenues.Add(&Venue);
			}
		}
	}
	for (int32 Index = 0; Index < KeyVenues.Num(); ++Index)
	{
		const FUEGT1TownVenueState& Venue = *KeyVenues[Index];
		const FVector2D Marker = WorldToMap(Venue.WorldLocation);
		FLinearColor MarkerColor = UEGT1Palette::Amber;
		if (Venue.VenueType == EUEGT1TownVenueType::FoodVenue) MarkerColor = FLinearColor(1.0f, 0.43f, 0.18f, 1.0f);
		else if (Venue.VenueType == EUEGT1TownVenueType::SocialVenue) MarkerColor = FLinearColor(0.20f, 0.84f, 0.96f, 1.0f);
		else if (Venue.VenueType == EUEGT1TownVenueType::Park) MarkerColor = FLinearColor(0.36f, 0.92f, 0.42f, 1.0f);
		DrawRect(FLinearColor::Black, Marker.X - 7.0f, Marker.Y - 7.0f, 14.0f, 14.0f);
		DrawRect(MarkerColor, Marker.X - 5.0f, Marker.Y - 5.0f, 10.0f, 10.0f);
		DrawText(FString::FromInt(Index + 1), FLinearColor::White, Marker.X + 7.0f, Marker.Y - 9.0f,
			GEngine->GetSmallFont(), 0.56f, false);
	}

	if (const APawn* Pawn = GetOwningPawn())
	{
		const FVector2D PlayerMarker = WorldToMap(Pawn->GetActorLocation());
		const FLinearColor PlayerColor(1.0f, 0.15f, 0.24f, 1.0f);
		DrawLine(PlayerMarker.X, PlayerMarker.Y - 10.0f, PlayerMarker.X + 9.0f, PlayerMarker.Y + 8.0f, PlayerColor, 4.0f);
		DrawLine(PlayerMarker.X + 9.0f, PlayerMarker.Y + 8.0f, PlayerMarker.X - 9.0f, PlayerMarker.Y + 8.0f, PlayerColor, 4.0f);
		DrawLine(PlayerMarker.X - 9.0f, PlayerMarker.Y + 8.0f, PlayerMarker.X, PlayerMarker.Y - 10.0f, PlayerColor, 4.0f);
		DrawText(TEXT("YOU"), FLinearColor::White, PlayerMarker.X + 12.0f, PlayerMarker.Y - 12.0f,
			GEngine->GetSmallFont(), 0.68f, false);
	}

	DrawText(TEXT("N"), UEGT1Palette::Paper, MapX + MapWidth - 28.0f, MapY + 14.0f,
		GEngine->GetMediumFont(), 0.9f, false);
	DrawLine(MapX + MapWidth - 21.0f, MapY + 42.0f, MapX + MapWidth - 21.0f, MapY + 68.0f,
		UEGT1Palette::Paper, 3.0f);
	DrawLine(MapX + MapWidth - 21.0f, MapY + 42.0f, MapX + MapWidth - 28.0f, MapY + 51.0f,
		UEGT1Palette::Paper, 3.0f);
	DrawLine(MapX + MapWidth - 21.0f, MapY + 42.0f, MapX + MapWidth - 14.0f, MapY + 51.0f,
		UEGT1Palette::Paper, 3.0f);

	DrawRect(FLinearColor(0.012f, 0.035f, 0.040f, 0.98f), DirectoryX, 92.0f, DirectoryWidth, ScreenHeight - 122.0f);
	DrawRect(UEGT1Palette::Signal, DirectoryX, 92.0f, 5.0f, ScreenHeight - 122.0f);
	DrawText(TEXT("KEY SERVICES + PLACES"), UEGT1Palette::Paper, DirectoryX + 18.0f, 108.0f,
		GEngine->GetMediumFont(), 0.86f, false);
	DrawText(FString::Printf(TEXT("%d residences shown as white squares"), HomeCount),
		FLinearColor(0.72f, 0.80f, 0.73f, 1.0f), DirectoryX + 18.0f, 138.0f,
		GEngine->GetSmallFont(), 0.68f, false);
	const int32 DirectoryRows = FMath::Max(1, FMath::DivideAndRoundUp(KeyVenues.Num(), 2));
	const float ColumnWidth = (DirectoryWidth - 32.0f) * 0.5f;
	const float RowHeight = FMath::Min(49.0f, (ScreenHeight - 245.0f) / DirectoryRows);
	for (int32 Index = 0; Index < KeyVenues.Num(); ++Index)
	{
		const int32 Column = Index / DirectoryRows;
		const int32 Row = Index % DirectoryRows;
		const float X = DirectoryX + 18.0f + Column * ColumnWidth;
		const float Y = 172.0f + Row * RowHeight;
		const FUEGT1TownVenueState& Venue = *KeyVenues[Index];
		DrawText(FString::Printf(TEXT("%02d  %s"), Index + 1, *Venue.DisplayName.Left(22)), UEGT1Palette::Paper,
			X, Y, GEngine->GetSmallFont(), 0.64f, false);
		const FString Detail = Venue.BusinessType.IsEmpty() ? FString(LexToString(Venue.VenueType)) : Venue.BusinessType;
		DrawText(Detail.Left(27), FLinearColor(0.58f, 0.76f, 0.65f, 1.0f), X + 25.0f, Y + 18.0f,
			GEngine->GetSmallFont(), 0.56f, false);
	}
	DrawText(TEXT("SERVICE   FOOD   SOCIAL   PARK   HOME"), FLinearColor(0.74f, 0.84f, 0.76f, 1.0f),
		DirectoryX + 18.0f, ScreenHeight - 66.0f, GEngine->GetSmallFont(), 0.60f, false);
	DrawText(TEXT("Full authored region  •  trails connect all Waystones"), FLinearColor(0.50f, 0.66f, 0.58f, 1.0f),
		DirectoryX + 18.0f, ScreenHeight - 44.0f, GEngine->GetSmallFont(), 0.57f, false);
}

void AUEGT1HUD::DrawPlayerStatusPanel(float ScreenWidth, float ScreenHeight)
{
	const UUEGT1TownSimulationSubsystem* Simulation = GetWorld()->GetSubsystem<UUEGT1TownSimulationSubsystem>();
	if (!Simulation || !Simulation->IsSimulationRunning())
	{
		return;
	}
	const FUEGT1TownSimulationModel& Model = Simulation->GetModel();
	const FUEGT1PlayerSimulationState& Player = Model.GetPlayer();
	const FDateTime Calendar = Model.GetCalendarDateTime();
	static const TCHAR* Weekdays[] = { TEXT("MON"), TEXT("TUE"), TEXT("WED"), TEXT("THU"), TEXT("FRI"), TEXT("SAT"), TEXT("SUN") };
	static const TCHAR* Months[] = { TEXT("JAN"), TEXT("FEB"), TEXT("MAR"), TEXT("APR"), TEXT("MAY"), TEXT("JUN"),
		TEXT("JUL"), TEXT("AUG"), TEXT("SEP"), TEXT("OCT"), TEXT("NOV"), TEXT("DEC") };
	const int32 WeekdayIndex = FMath::Clamp(static_cast<int32>(Calendar.GetDayOfWeek()), 0, 6);
	const FString CalendarLabel = FString::Printf(TEXT("%s %s %02d %04d  %02d:%02d"), Weekdays[WeekdayIndex],
		Months[Calendar.GetMonth() - 1], Calendar.GetDay(), Calendar.GetYear(), Calendar.GetHour(), Calendar.GetMinute());
	const APawn* Pawn = GetOwningPawn();
	const FVector Position = Pawn ? Pawn->GetActorLocation() : FVector::ZeroVector;
	const float Temperature = UEGT1WorldLayout::GetTemperatureCelsius(Position, Model.GetHourOfDay());
	const float Width = 435.0f;
	const float X = FMath::Max(30.0f, ScreenWidth - Width - 30.0f);
	const float Y = 28.0f;
	DrawRect(FLinearColor(0.01f, 0.035f, 0.03f, 0.92f), X, Y, Width, 126.0f);
	DrawRect(UEGT1Palette::Signal, X, Y, 6.0f, 126.0f);
	DrawText(CalendarLabel, UEGT1Palette::Paper,
		X + 18.0f, Y + 13.0f, GEngine->GetSmallFont(), 0.90f, false);
	DrawText(FString::Printf(TEXT("%.0f C   CASH  $%.0f"), Temperature, Player.Money), UEGT1Palette::Amber,
		X + 18.0f, Y + 38.0f, GEngine->GetMediumFont(), 0.92f, false);
	DrawText(FString::Printf(TEXT("ENERGY %3.0f%%   FOOD %3.0f%%   HYGIENE %3.0f%%   SOCIAL %3.0f%%"),
		Player.Needs.Energy * 100.0f, Player.Needs.Hunger * 100.0f, Player.Needs.Hygiene * 100.0f,
		Player.Needs.Social * 100.0f), UEGT1Palette::Paper, X + 18.0f, Y + 72.0f,
		GEngine->GetSmallFont(), 0.73f, false);
	DrawText(Player.LastActivityMessage.Left(70), FLinearColor(0.65f, 0.80f, 0.70f, 1.0f),
		X + 18.0f, Y + 99.0f, GEngine->GetSmallFont(), 0.72f, false);
}

void AUEGT1HUD::DrawResidentThoughtBubbles(float ScreenWidth, float ScreenHeight)
{
	const UUEGT1TownSimulationSubsystem* Simulation = GetWorld()->GetSubsystem<UUEGT1TownSimulationSubsystem>();
	APlayerController* PlayerController = GetOwningPlayerController();
	if (!Simulation || !Simulation->IsSimulationRunning() || !PlayerController)
	{
		return;
	}

	FVector ViewLocation;
	FRotator ViewRotation;
	PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
	const FUEGT1SimulationTuning Tuning = UUEGT1TownSimulationSettings::Get().MakeTuning();
	const float MaximumDistanceSquared = FMath::Square(Tuning.ThoughtBubbleVisibleDistance);
	struct FThoughtBubbleDraw
	{
		float DistanceSquared = 0.0f;
		FVector2D ScreenPosition = FVector2D::ZeroVector;
		FString Text;
	};
	TArray<FThoughtBubbleDraw> Bubbles;
	const TArray<FUEGT1NPCSimulationState>& NPCs = Simulation->GetModel().GetNPCs();
	for (int32 NPCIndex = 0; NPCIndex < NPCs.Num(); ++NPCIndex)
	{
		FVector ResidentLocation;
		if (NPCs[NPCIndex].CurrentThought.IsEmpty() || !Simulation->GetResidentPresentationLocation(NPCIndex, ResidentLocation))
		{
			continue;
		}
		const float DistanceSquared = FVector::DistSquared(ViewLocation, ResidentLocation);
		if (DistanceSquared > MaximumDistanceSquared)
		{
			continue;
		}
		FVector2D ScreenPosition;
		if (!PlayerController->ProjectWorldLocationToScreen(ResidentLocation + FVector(0.0f, 0.0f, 235.0f), ScreenPosition, true) ||
			ScreenPosition.X < 0.0f || ScreenPosition.X > ScreenWidth || ScreenPosition.Y < 0.0f || ScreenPosition.Y > ScreenHeight)
		{
			continue;
		}
		FThoughtBubbleDraw& Bubble = Bubbles.AddDefaulted_GetRef();
		Bubble.DistanceSquared = DistanceSquared;
		Bubble.ScreenPosition = ScreenPosition;
		Bubble.Text = NPCs[NPCIndex].CurrentThought.Left(64);
	}
	Bubbles.Sort([](const FThoughtBubbleDraw& A, const FThoughtBubbleDraw& B)
	{
		return A.DistanceSquared < B.DistanceSquared;
	});

	for (int32 Index = 0; Index < FMath::Min(Tuning.MaxVisibleThoughtBubbles, Bubbles.Num()); ++Index)
	{
		const FThoughtBubbleDraw& Bubble = Bubbles[Index];
		float TextWidth = 0.0f;
		float TextHeight = 0.0f;
		GetTextSize(Bubble.Text, TextWidth, TextHeight, GEngine->GetSmallFont(), 0.76f);
		const float BubbleWidth = FMath::Clamp(TextWidth + 26.0f, 150.0f, 410.0f);
		const float BubbleHeight = TextHeight + 18.0f;
		const float X = FMath::Clamp(Bubble.ScreenPosition.X - BubbleWidth * 0.5f, 8.0f, ScreenWidth - BubbleWidth - 8.0f);
		const float Y = FMath::Clamp(Bubble.ScreenPosition.Y - BubbleHeight, 170.0f, ScreenHeight - BubbleHeight - 8.0f);
		DrawRect(FLinearColor(0.01f, 0.035f, 0.03f, 0.91f), X, Y, BubbleWidth, BubbleHeight);
		DrawRect(UEGT1Palette::Amber, X, Y, 4.0f, BubbleHeight);
		DrawText(Bubble.Text, UEGT1Palette::Paper, X + 13.0f, Y + 8.0f, GEngine->GetSmallFont(), 0.76f, false);
	}
}

void AUEGT1HUD::DrawObjectivePanel(float ScreenWidth, float ScreenHeight)
{
	const bool bTechDemo = UGameplayStatics::GetCurrentLevelName(GetWorld(), true).Contains(TEXT("TechDemo"));
	const AUEGT1MilestoneGameState* State = GetWorld()->GetGameState<AUEGT1MilestoneGameState>();
	const int32 Activated = State ? State->GetActivatedCount() : 0;
	const int32 Total = State ? State->GetTotalCount() : 3;
	const bool bComplete = State && State->IsMilestoneComplete();

	DrawRect(UEGT1Palette::Ink, 30.0f, 28.0f, bTechDemo ? 430.0f : 370.0f, 105.0f);
	DrawRect(bComplete ? UEGT1Palette::Signal : UEGT1Palette::Amber, 30.0f, 28.0f, 6.0f, 105.0f);
	DrawText(bTechDemo ? TEXT("LUMEN WILDS") : TEXT("SIGNAL GROVE"), UEGT1Palette::Paper, 52.0f, 43.0f, GEngine->GetMediumFont(), 1.0f, false);
	DrawText(bTechDemo ? TEXT("UE5 real-time environment showcase") : (bComplete ? TEXT("Sanctuary restored") : TEXT("Restore the three Waystones")),
		bComplete ? UEGT1Palette::Signal : UEGT1Palette::Paper, 52.0f, 75.0f, GEngine->GetSmallFont(), 1.05f, false);
	DrawText(bTechDemo ? TEXT("EXPLORE  •  FLY  •  INSPECT") : FString::Printf(TEXT("SIGNALS  %d / %d"), Activated, FMath::Max(Total, 3)),
		bComplete ? UEGT1Palette::Signal : UEGT1Palette::Amber, 52.0f, 101.0f, GEngine->GetSmallFont(), 0.9f, false);

	if (GetWorld()->GetTimeSeconds() < 12.0f)
	{
		DrawText(TEXT("WASD move   SHIFT sprint   SPACE jump/up   CTRL down   M map   F3 diagnostics   F4 resident   F5 save   F6 load   ESC menu"),
			FLinearColor(0.72f, 0.82f, 0.73f, 0.95f), 32.0f, ScreenHeight - 42.0f, GEngine->GetSmallFont(), 0.85f, false);
	}

	if (bComplete && !bTechDemo)
	{
		const FString CompletionText = TEXT("THE GROVE ANSWERS");
		float TextWidth = 0.0f;
		float TextHeight = 0.0f;
		GetTextSize(CompletionText, TextWidth, TextHeight, GEngine->GetLargeFont(), 1.0f);
		DrawRect(FLinearColor(0.01f, 0.08f, 0.075f, 0.86f), ScreenWidth * 0.5f - 260.0f, 70.0f, 520.0f, 62.0f);
		DrawText(CompletionText, UEGT1Palette::Signal, ScreenWidth * 0.5f - TextWidth * 0.5f, 83.0f, GEngine->GetLargeFont(), 1.0f, false);
	}
}

void AUEGT1HUD::DrawDeveloperModePanel(float ScreenWidth, float ScreenHeight)
{
	const AUEGT1ExplorerCharacter* Character = Cast<AUEGT1ExplorerCharacter>(GetOwningPawn());
	if (!Character || !Character->IsDeveloperModeEnabled())
	{
		return;
	}

	const FString State = Character->IsDeveloperFlying()
		? TEXT("DEV MODE  •  INVINCIBLE  •  4.2K SPEED  •  FLIGHT")
		: TEXT("DEV MODE  •  INVINCIBLE  •  4.2K SPEED  •  F9 TO FLY");
	float Width = 0.0f;
	float Height = 0.0f;
	GetTextSize(State, Width, Height, GEngine->GetSmallFont(), 0.9f);
	DrawRect(FLinearColor(0.12f, 0.015f, 0.04f, 0.90f), ScreenWidth * 0.5f - Width * 0.5f - 18.0f, 28.0f, Width + 36.0f, 34.0f);
	DrawRect(FLinearColor(1.0f, 0.15f, 0.35f, 1.0f), ScreenWidth * 0.5f - Width * 0.5f - 18.0f, 28.0f, 5.0f, 34.0f);
	DrawText(State, FLinearColor(1.0f, 0.72f, 0.77f, 1.0f), ScreenWidth * 0.5f - Width * 0.5f, 37.0f, GEngine->GetSmallFont(), 0.9f, false);
}

void AUEGT1HUD::DrawInteractionPrompt(float ScreenWidth, float ScreenHeight)
{
	const AUEGT1ExplorerCharacter* Character = Cast<AUEGT1ExplorerCharacter>(GetOwningPawn());
	const UUEGT1InteractionComponent* Interaction = Character ? Character->GetInteractionComponent() : nullptr;
	if (!Interaction || !Interaction->HasValidFocus())
	{
		return;
	}

	const FString Prompt = Interaction->GetFocusedPrompt().ToString();
	float Width = 0.0f;
	float Height = 0.0f;
	GetTextSize(Prompt, Width, Height, GEngine->GetMediumFont(), 1.0f);
	DrawRect(FLinearColor(0.01f, 0.04f, 0.035f, 0.88f), ScreenWidth * 0.5f - Width * 0.5f - 18.0f, ScreenHeight * 0.70f - 10.0f, Width + 36.0f, Height + 20.0f);
	DrawText(Prompt, UEGT1Palette::Paper, ScreenWidth * 0.5f - Width * 0.5f, ScreenHeight * 0.70f, GEngine->GetMediumFont(), 1.0f, false);
}

void AUEGT1HUD::DrawCrosshair(float ScreenWidth, float ScreenHeight)
{
	const float CenterX = ScreenWidth * 0.5f;
	const float CenterY = ScreenHeight * 0.5f;
	const FLinearColor Color(0.82f, 0.93f, 0.84f, 0.85f);
	DrawRect(Color, CenterX - 1.0f, CenterY - 8.0f, 2.0f, 5.0f);
	DrawRect(Color, CenterX - 1.0f, CenterY + 3.0f, 2.0f, 5.0f);
	DrawRect(Color, CenterX - 8.0f, CenterY - 1.0f, 5.0f, 2.0f);
	DrawRect(Color, CenterX + 3.0f, CenterY - 1.0f, 5.0f, 2.0f);
}

void AUEGT1HUD::DrawDiagnostics(float ScreenWidth, float ScreenHeight)
{
	const APawn* Pawn = GetOwningPawn();
	const AUEGT1ExplorerCharacter* Character = Cast<AUEGT1ExplorerCharacter>(Pawn);
	const UUEGT1InteractionComponent* Interaction = Character ? Character->GetInteractionComponent() : nullptr;
	const float Fps = GetWorld()->GetDeltaSeconds() > SMALL_NUMBER ? 1.0f / GetWorld()->GetDeltaSeconds() : 0.0f;
	const FVector Position = Pawn ? Pawn->GetActorLocation() : FVector::ZeroVector;
	const FUEGT1RegionSample Region = UEGT1WorldLayout::SampleRegion(Position);
	const FString Focus = Interaction && Interaction->GetFocusedActor() ? Interaction->GetFocusedActor()->GetName() : TEXT("None");

	const float Y = 166.0f;
	DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.80f), ScreenWidth - 415.0f, Y, 385.0f, 178.0f);
	DrawText(TEXT("UEGT1 REGION DIAGNOSTICS"), UEGT1Palette::Signal, ScreenWidth - 397.0f, Y + 14.0f, GEngine->GetSmallFont(), 0.9f, false);
	DrawText(FString::Printf(TEXT("FPS %.0f   Seed %d"), Fps, UEGT1WorldLayout::GetWorldSeed()), UEGT1Palette::Paper, ScreenWidth - 397.0f, Y + 40.0f, GEngine->GetSmallFont(), 0.85f, false);
	DrawText(FString::Printf(TEXT("XYZ %.0f  %.0f  %.0f"), Position.X, Position.Y, Position.Z), UEGT1Palette::Paper, ScreenWidth - 397.0f, Y + 63.0f, GEngine->GetSmallFont(), 0.85f, false);
	DrawText(FString::Printf(TEXT("Region %s   Ground %.0f   Water %.0f"), LexToString(Region.GetDominantBiome()), Region.SurfaceHeight, Region.WaterDepth),
		UEGT1Palette::Paper, ScreenWidth - 397.0f, Y + 86.0f, GEngine->GetSmallFont(), 0.82f, false);
	DrawText(FString::Printf(TEXT("Temp %.2f   Moisture %.2f"), Region.Temperature, Region.Moisture),
		UEGT1Palette::Paper, ScreenWidth - 397.0f, Y + 109.0f, GEngine->GetSmallFont(), 0.82f, false);
	DrawText(FString::Printf(TEXT("Focus %s"), *Focus), UEGT1Palette::Paper, ScreenWidth - 397.0f, Y + 132.0f, GEngine->GetSmallFont(), 0.82f, false);
	DrawText(TEXT("Trace: uegt1.Debug.DrawInteraction 1"), FLinearColor(0.65f, 0.72f, 0.67f, 1.0f), ScreenWidth - 397.0f, Y + 155.0f, GEngine->GetSmallFont(), 0.72f, false);
}

void AUEGT1HUD::DrawSimulationInspector(float ScreenWidth, float ScreenHeight)
{
	const UUEGT1TownSimulationSubsystem* Simulation = GetWorld()->GetSubsystem<UUEGT1TownSimulationSubsystem>();
	if (!Simulation || !Simulation->IsSimulationRunning())
	{
		return;
	}
	const FUEGT1TownSimulationModel& Model = Simulation->GetModel();
	const FUEGT1NPCSimulationState NPC = Simulation->GetInspectedNPC();
	const FUEGT1TownVenueState* PreferredJob = Model.GetVenues().FindByPredicate([&NPC](const FUEGT1TownVenueState& Venue)
	{
		return Venue.VenueId == NPC.JobId;
	});
	FString HouseholdSummary = TEXT("Lives alone");
	if (!NPC.HouseholdRelationships.IsEmpty())
	{
		HouseholdSummary = FString::Printf(TEXT("%s: "), LexToString(NPC.HouseholdRelationships[0].Relationship));
		for (int32 Index = 0; Index < NPC.HouseholdRelationships.Num(); ++Index)
		{
			const FName OtherId = NPC.HouseholdRelationships[Index].OtherNpcId;
			const FUEGT1NPCSimulationState* Other = Model.GetNPCs().FindByPredicate([OtherId](const FUEGT1NPCSimulationState& Candidate)
			{
				return Candidate.NpcId == OtherId;
			});
			HouseholdSummary += Other ? Other->DisplayName : OtherId.ToString();
			HouseholdSummary += Index + 1 < NPC.HouseholdRelationships.Num() ? TEXT(", ") : TEXT("");
		}
	}
	const float X = FMath::Max(30.0f, ScreenWidth - 530.0f);
	const float Y = 356.0f;
	DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.84f), X, Y, 500.0f, 380.0f);
	DrawText(TEXT("TOWN SIMULATION INSPECTOR  [F4 NEXT]"), UEGT1Palette::Signal, X + 18.0f, Y + 14.0f,
		GEngine->GetSmallFont(), 0.86f, false);
	DrawText(FString::Printf(TEXT("Day %d  %02d:%02d   Seed %d   Residents %d"), Model.GetDayIndex() + 1,
		FMath::FloorToInt(Model.GetHourOfDay()), FMath::FloorToInt(FMath::Fmod(Model.GetAbsoluteMinutes(), 60.0f)),
		Model.GetSeed(), Model.GetNPCs().Num()), UEGT1Palette::Paper, X + 18.0f, Y + 39.0f, GEngine->GetSmallFont(), 0.82f, false);
	DrawText(FString::Printf(TEXT("%s [%s]   $%.0f"), *NPC.DisplayName, *NPC.NpcId.ToString(), NPC.Money),
		UEGT1Palette::Amber, X + 18.0f, Y + 64.0f, GEngine->GetSmallFont(), 0.86f, false);
	DrawText(FString::Printf(TEXT("Needs  Energy %.2f  Hunger %.2f  Hygiene %.2f  Social %.2f"),
		NPC.Needs.Energy, NPC.Needs.Hunger, NPC.Needs.Hygiene, NPC.Needs.Social),
		UEGT1Palette::Paper, X + 18.0f, Y + 89.0f, GEngine->GetSmallFont(), 0.78f, false);
	DrawText(FString::Printf(TEXT("Home %s   Bed %s"), *NPC.HomeId.ToString(), *NPC.BedId.ToString()),
		UEGT1Palette::Paper, X + 18.0f, Y + 112.0f, GEngine->GetSmallFont(), 0.78f, false);
	DrawText(HouseholdSummary.Left(76), UEGT1Palette::Paper, X + 18.0f, Y + 135.0f, GEngine->GetSmallFont(), 0.76f, false);
	DrawText(FString::Printf(TEXT("Preferred job %s   $%.0f/hr   open %.0f-%.0f"),
		PreferredJob ? *PreferredJob->DisplayName : TEXT("None"), PreferredJob ? PreferredJob->HourlyRate : 0.0f,
		PreferredJob ? PreferredJob->OpeningHour : 0.0f, PreferredJob ? PreferredJob->ClosingHour : 0.0f),
		UEGT1Palette::Paper, X + 18.0f, Y + 158.0f, GEngine->GetSmallFont(), 0.76f, false);
	DrawText(FString::Printf(TEXT("Goal $%.0f   willing %.0fh   now %.0fh   total %.0fh   sleep %.1f-%.1f"), NPC.SavingsGoal,
		NPC.PreferredWorkSessionHours, NPC.CurrentWorkSessionMinutes / 60.0f, NPC.TotalWorkMinutes / 60.0f,
		NPC.Schedule.SleepStartHour, NPC.Schedule.SleepEndHour),
		UEGT1Palette::Paper, X + 18.0f, Y + 181.0f, GEngine->GetSmallFont(), 0.74f, false);
	DrawText(FString::Printf(TEXT("Action %s -> %s   %.0f min remain"), LexToString(NPC.CurrentAction),
		NPC.DestinationId.IsNone() ? TEXT("None") : *NPC.DestinationId.ToString(), NPC.RemainingActionMinutes),
		UEGT1Palette::Signal, X + 18.0f, Y + 206.0f, GEngine->GetSmallFont(), 0.82f, false);
	DrawText(FString::Printf(TEXT("Thought: %s"), *NPC.CurrentThought.Left(70)), UEGT1Palette::Amber,
		X + 18.0f, Y + 231.0f, GEngine->GetSmallFont(), 0.74f, false);
	for (int32 Index = 0; Index < FMath::Min(3, NPC.UtilityScores.Num()); ++Index)
	{
		const FUEGT1ActionUtilityScore& Utility = NPC.UtilityScores[Index];
		DrawText(FString::Printf(TEXT("U%d  %s  %.1f  %s"), Index + 1, LexToString(Utility.Action), Utility.Score,
			*Utility.DestinationId.ToString()), FLinearColor(0.72f, 0.82f, 0.73f, 1.0f),
			X + 18.0f, Y + 258.0f + Index * 22.0f, GEngine->GetSmallFont(), 0.74f, false);
	}
	const FString Failure = NPC.LatestFailureReason.IsEmpty() ? TEXT("none") : NPC.LatestFailureReason.Left(72);
	DrawText(FString::Printf(TEXT("Latest replan: %s"), *Failure), FLinearColor(0.95f, 0.63f, 0.36f, 1.0f),
		X + 18.0f, Y + 329.0f, GEngine->GetSmallFont(), 0.72f, false);
	DrawText(TEXT("F5 save simulation   F6 restore simulation"), FLinearColor(0.62f, 0.70f, 0.65f, 1.0f),
		X + 18.0f, Y + 351.0f, GEngine->GetSmallFont(), 0.70f, false);
}
