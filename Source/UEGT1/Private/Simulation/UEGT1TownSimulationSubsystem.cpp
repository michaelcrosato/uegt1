#include "Simulation/UEGT1TownSimulationSubsystem.h"

#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/SkyLight.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Simulation/UEGT1TownDestinationComponent.h"
#include "Simulation/UEGT1TownResident.h"
#include "Simulation/UEGT1TownSimulationSaveGame.h"
#include "Simulation/UEGT1TownSimulationSettings.h"
#include "UEGT1LogChannels.h"
#include "World/UEGT1Town.h"
#include "World/UEGT1DayNight.h"

void UUEGT1TownSimulationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	bShuttingDown = false;
}

void UUEGT1TownSimulationSubsystem::Deinitialize()
{
	bShuttingDown = true;
	RegisteredDestinations.Reset();
	ResidentActors.Reset();
	SimulationSun.Reset();
	SimulationSkyLight.Reset();
	SimulationFog.Reset();
	SimulationExposure.Reset();
	Super::Deinitialize();
}

bool UUEGT1TownSimulationSubsystem::IsTickable() const
{
	return !IsTemplate() && !bShuttingDown && Model.IsInitialized();
}

TStatId UUEGT1TownSimulationSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UUEGT1TownSimulationSubsystem, STATGROUP_Tickables);
}

void UUEGT1TownSimulationSubsystem::Tick(float DeltaTime)
{
	const FUEGT1SimulationTuning Tuning = UUEGT1TownSimulationSettings::Get().MakeTuning();
	const float BatchInterval = FMath::Max(Tuning.SimulationBatchIntervalRealSeconds, 1.0f / 60.0f);
	RealTimeAccumulator += DeltaTime;
	bool bAdvancedSimulation = false;
	int32 CatchUpSteps = 0;
	while (RealTimeAccumulator >= BatchInterval && CatchUpSteps < 8)
	{
		Model.AdvanceMinutes(BatchInterval * FMath::Max(Tuning.SimMinutesPerRealSecond, 0.0f));
		SyncResidentActorTargets(BatchInterval);
		RealTimeAccumulator -= BatchInterval;
		bAdvancedSimulation = true;
		++CatchUpSteps;
	}
	AdvanceResidentVisuals(DeltaTime);
	if (bAdvancedSimulation)
	{
		UpdateDayNightLighting();
	}
}

void UUEGT1TownSimulationSubsystem::RegisterDestination(UUEGT1TownDestinationComponent* Destination)
{
	if (Destination)
	{
		RegisteredDestinations.AddUnique(Destination);
	}
}

void UUEGT1TownSimulationSubsystem::UnregisterDestination(UUEGT1TownDestinationComponent* Destination)
{
	RegisteredDestinations.Remove(Destination);
}

void UUEGT1TownSimulationSubsystem::StartSimulation(int32 RequestedSeed)
{
	if (Model.IsInitialized())
	{
		return;
	}
	const UUEGT1TownSimulationSettings& Settings = UUEGT1TownSimulationSettings::Get();
	int32 Seed = RequestedSeed != 0 ? RequestedSeed : Settings.TownSeed;
	int32 NPCCount = Settings.NPCCount;
	FParse::Value(FCommandLine::Get(), TEXT("UEGT1TownSeed="), Seed);
	FParse::Value(FCommandLine::Get(), TEXT("UEGT1TownNPCs="), NPCCount);
	NPCCount = FMath::Clamp(NPCCount, 100, 100);

	TArray<FUEGT1TownVenueState> VenueStates;
	for (const TWeakObjectPtr<UUEGT1TownDestinationComponent>& Destination : RegisteredDestinations)
	{
		if (Destination.IsValid())
		{
			VenueStates.Add(Destination->MakeVenueState());
		}
	}
	VenueStates.Sort([](const FUEGT1TownVenueState& A, const FUEGT1TownVenueState& B)
	{
		return A.VenueId.LexicalLess(B.VenueId);
	});
	Model.Initialize(Seed, NPCCount, VenueStates, Settings.MakeTuning());
	if (!Model.IsInitialized())
	{
		UE_LOG(LogUEGT1, Error, TEXT("Town simulation could not start: generated homes do not provide one unique bed per requested citizen."));
		return;
	}
	SpawnOrRefreshResidentActors();
	UpdateDayNightLighting();
	const int32 AssignedBedCount = Model.HasCompleteBedAssignments() ? Model.GetNPCs().Num() : 0;
	const int32 RelationshipCount = Model.HasCompleteHouseholdRelationships() ? Model.GetNPCs().Num() : 0;
	int32 JobCount = 0;
	for (const FUEGT1TownVenueState& Venue : Model.GetVenues())
	{
		JobCount += Venue.VenueType == EUEGT1TownVenueType::Workplace ? 1 : 0;
	}
	UE_LOG(LogUEGT1, Display, TEXT("Town simulation ready: Seed=%d NPCs=%d Spawned=%d Venues=%d Jobs=%d Beds=%d AssignedBeds=%d RelatedHouseholds=%d Time=%02d:%02d CitizenSpeed=%.0f PlayerWalkSpeed=480 Batched=true Presentation=Interpolated ThoughtBubbles=Enabled"),
		Seed, Model.GetNPCs().Num(), ResidentActors.Num(), Model.GetVenues().Num(), JobCount, Model.GetBedCount(), AssignedBedCount, RelationshipCount,
		FMath::FloorToInt(Model.GetHourOfDay()),
		FMath::FloorToInt(FMath::Fmod(Model.GetAbsoluteMinutes(), 60.0f)), Settings.MakeTuning().CitizenWalkSpeedCentimetersPerSecond);
}

void UUEGT1TownSimulationSubsystem::AdvanceSimulationMinutes(float Minutes)
{
	Model.AdvanceMinutes(Minutes);
	SyncResidentActorTargets(0.0f);
	UpdateDayNightLighting();
}

bool UUEGT1TownSimulationSubsystem::CanPlayerPerformActivity(EUEGT1SimActionType Action, FName VenueId,
	FString& OutReason) const
{
	return Model.IsInitialized() && Model.CanPlayerPerformActivity(Action, VenueId, OutReason);
}

bool UUEGT1TownSimulationSubsystem::PerformPlayerActivity(EUEGT1SimActionType Action, FName VenueId,
	FString& OutResult)
{
	if (!Model.PerformPlayerActivity(Action, VenueId, OutResult))
	{
		return false;
	}
	SyncResidentActorTargets(0.0f);
	UpdateDayNightLighting();
	return true;
}

bool UUEGT1TownSimulationSubsystem::SaveSimulation(const FString& SlotName)
{
	if (!Model.IsInitialized())
	{
		return false;
	}
	UUEGT1TownSimulationSaveGame* Save = Cast<UUEGT1TownSimulationSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UUEGT1TownSimulationSaveGame::StaticClass()));
	if (!Save)
	{
		return false;
	}
	Save->Snapshot = Model.MakeSnapshot();
	const FString ResolvedSlot = ResolveSaveSlot(SlotName);
	const bool bSaved = UGameplayStatics::SaveGameToSlot(Save, ResolvedSlot, 0);
	if (bSaved)
	{
		UE_LOG(LogUEGT1, Display, TEXT("Town simulation save: Slot=%s Success=true Day=%d Time=%.2f NPCs=%d"),
			*ResolvedSlot, Model.GetDayIndex(), Model.GetHourOfDay(), Model.GetNPCs().Num());
	}
	else
	{
		UE_LOG(LogUEGT1, Error, TEXT("Town simulation save: Slot=%s Success=false"), *ResolvedSlot);
	}
	return bSaved;
}

bool UUEGT1TownSimulationSubsystem::LoadSimulation(const FString& SlotName)
{
	const FString ResolvedSlot = ResolveSaveSlot(SlotName);
	const UUEGT1TownSimulationSaveGame* Save = Cast<UUEGT1TownSimulationSaveGame>(UGameplayStatics::LoadGameFromSlot(ResolvedSlot, 0));
	if (!Save || !Model.Restore(Save->Snapshot, UUEGT1TownSimulationSettings::Get().MakeTuning()))
	{
		UE_LOG(LogUEGT1, Warning, TEXT("Town simulation load failed: Slot=%s"), *ResolvedSlot);
		return false;
	}
	for (TActorIterator<AUEGT1Town> Iterator(GetWorld()); Iterator; ++Iterator)
	{
		Iterator->RegenerateFromSimulationSeed(Model.GetSeed());
		break;
	}
	SpawnOrRefreshResidentActors();
	UpdateDayNightLighting();
	UE_LOG(LogUEGT1, Display, TEXT("Town simulation restored: Slot=%s Seed=%d Day=%d Time=%.2f NPCs=%d"),
		*ResolvedSlot, Model.GetSeed(), Model.GetDayIndex(), Model.GetHourOfDay(), Model.GetNPCs().Num());
	return true;
}

void UUEGT1TownSimulationSubsystem::CycleInspectedNPC(int32 Direction)
{
	const int32 Count = Model.GetNPCs().Num();
	if (Count > 0)
	{
		InspectedNPCIndex = (InspectedNPCIndex + Direction) % Count;
		if (InspectedNPCIndex < 0)
		{
			InspectedNPCIndex += Count;
		}
	}
}

FUEGT1NPCSimulationState UUEGT1TownSimulationSubsystem::GetInspectedNPC() const
{
	static const FUEGT1NPCSimulationState Empty;
	return Model.GetNPCs().IsValidIndex(InspectedNPCIndex) ? Model.GetNPCs()[InspectedNPCIndex] : Empty;
}

void UUEGT1TownSimulationSubsystem::SpawnOrRefreshResidentActors()
{
	for (const TWeakObjectPtr<AUEGT1TownResident>& Resident : ResidentActors)
	{
		if (Resident.IsValid())
		{
			Resident->Destroy();
		}
	}
	ResidentActors.Reset();
	for (const FUEGT1NPCSimulationState& NPC : Model.GetNPCs())
	{
		AUEGT1TownResident* Resident = GetWorld()->SpawnActor<AUEGT1TownResident>(
			AUEGT1TownResident::StaticClass(), NPC.WorldLocation, FRotator::ZeroRotator);
		if (Resident)
		{
			Resident->InitializeResident(NPC.NpcId, Model.GetSeed());
			Resident->ApplySimulationPosition(NPC.WorldLocation);
			ResidentActors.Add(Resident);
		}
	}
}

void UUEGT1TownSimulationSubsystem::SyncResidentActorTargets(float BlendDurationSeconds)
{
	const TArray<FUEGT1NPCSimulationState>& NPCs = Model.GetNPCs();
	for (int32 Index = 0; Index < NPCs.Num() && Index < ResidentActors.Num(); ++Index)
	{
		if (ResidentActors[Index].IsValid())
		{
			if (BlendDurationSeconds <= SMALL_NUMBER)
			{
				ResidentActors[Index]->ApplySimulationPosition(NPCs[Index].WorldLocation);
			}
			else
			{
				ResidentActors[Index]->SetSimulationTarget(NPCs[Index].WorldLocation, BlendDurationSeconds);
			}
		}
	}
}

bool UUEGT1TownSimulationSubsystem::GetResidentPresentationLocation(int32 NPCIndex, FVector& OutLocation) const
{
	if (ResidentActors.IsValidIndex(NPCIndex) && ResidentActors[NPCIndex].IsValid())
	{
		OutLocation = ResidentActors[NPCIndex]->GetActorLocation();
		return true;
	}
	return false;
}

void UUEGT1TownSimulationSubsystem::AdvanceResidentVisuals(float DeltaTime)
{
	for (const TWeakObjectPtr<AUEGT1TownResident>& Resident : ResidentActors)
	{
		if (Resident.IsValid())
		{
			const FVector Before = Resident->GetActorLocation();
			const FVector Target = Resident->GetVisualTargetPosition();
			const bool bWasMoving = Resident->IsVisualMovementActive();
			Resident->AdvanceVisualMovement(DeltaTime);
			const FVector Sample = Resident->GetActorLocation();
			if (!bLoggedVisualInterpolation && bWasMoving && !Sample.Equals(Before, 0.1f) && !Sample.Equals(Target, 0.1f))
			{
				UE_LOG(LogUEGT1, Display, TEXT("Resident visual interpolation observed: NPC=%s Start=%s Sample=%s Target=%s"),
					*Resident->GetNpcId().ToString(), *Before.ToCompactString(), *Sample.ToCompactString(), *Target.ToCompactString());
				bLoggedVisualInterpolation = true;
			}
		}
	}
}

void UUEGT1TownSimulationSubsystem::UpdateDayNightLighting()
{
	if (!SimulationSun.IsValid())
	{
		ADirectionalLight* FirstDirectionalLight = nullptr;
		for (TActorIterator<ADirectionalLight> Iterator(GetWorld()); Iterator; ++Iterator)
		{
			FirstDirectionalLight = FirstDirectionalLight ? FirstDirectionalLight : *Iterator;
			if (Iterator->GetName().Contains(TEXT("Signal_Grove_Sun")) || Iterator->GetName().Contains(TEXT("SignalGroveSun")))
			{
				SimulationSun = *Iterator;
				break;
			}
		}
		SimulationSun = SimulationSun.IsValid() ? SimulationSun : FirstDirectionalLight;
	}
	if (!SimulationSkyLight.IsValid())
	{
		ASkyLight* FirstSkyLight = nullptr;
		for (TActorIterator<ASkyLight> Iterator(GetWorld()); Iterator; ++Iterator)
		{
			FirstSkyLight = FirstSkyLight ? FirstSkyLight : *Iterator;
			if (Iterator->GetName().Contains(TEXT("Signal_Grove_Sky_Light")) || Iterator->GetName().Contains(TEXT("SignalGroveSkyLight")))
			{
				SimulationSkyLight = *Iterator;
				break;
			}
		}
		SimulationSkyLight = SimulationSkyLight.IsValid() ? SimulationSkyLight : FirstSkyLight;
	}
	if (!SimulationFog.IsValid())
	{
		AExponentialHeightFog* FirstFog = nullptr;
		for (TActorIterator<AExponentialHeightFog> Iterator(GetWorld()); Iterator; ++Iterator)
		{
			FirstFog = FirstFog ? FirstFog : *Iterator;
			if (Iterator->GetName().Contains(TEXT("Signal_Grove_Mist")) || Iterator->GetName().Contains(TEXT("SignalGroveMist")))
			{
				SimulationFog = *Iterator;
				break;
			}
		}
		SimulationFog = SimulationFog.IsValid() ? SimulationFog : FirstFog;
	}
	if (!SimulationExposure.IsValid())
	{
		for (TActorIterator<APostProcessVolume> Iterator(GetWorld()); Iterator; ++Iterator)
		{
			SimulationExposure = *Iterator;
			if (Iterator->GetName().Contains(TEXT("Signal_Grove_Exposure")) || Iterator->GetName().Contains(TEXT("SignalGroveExposure")))
			{
				break;
			}
		}
	}

	const FUEGT1DayNightState Lighting = UEGT1DayNight::Evaluate(Model.GetHourOfDay());
	if (SimulationSun.IsValid())
	{
		SimulationSun->SetActorRotation(FRotator(-Lighting.SunElevationDegrees, Lighting.SunAzimuthDegrees, 0.0f));
		if (UDirectionalLightComponent* SunComponent = Cast<UDirectionalLightComponent>(SimulationSun->GetLightComponent()))
		{
			SunComponent->SetIntensity(Lighting.SunIntensityLux);
			SunComponent->SetLightColor(Lighting.SunColor);
			SunComponent->SetCastShadows(Lighting.DaylightAlpha > 0.01f);
		}
	}
	if (SimulationSkyLight.IsValid())
	{
		if (USkyLightComponent* SkyComponent = SimulationSkyLight->GetLightComponent())
		{
			SkyComponent->SetIntensity(Lighting.SkyLightIntensity);
			SkyComponent->SetLightColor(Lighting.SkyColor);
		}
	}
	if (SimulationFog.IsValid())
	{
		if (UExponentialHeightFogComponent* FogComponent = SimulationFog->GetComponent())
		{
			FogComponent->SetFogInscatteringColor(Lighting.FogColor);
			FogComponent->SetDirectionalInscatteringColor(Lighting.FogColor);
		}
	}
	if (SimulationExposure.IsValid())
	{
		SimulationExposure->Settings.bOverride_AutoExposureMinBrightness = true;
		SimulationExposure->Settings.bOverride_AutoExposureMaxBrightness = true;
		SimulationExposure->Settings.AutoExposureMinBrightness = Lighting.ExposureEV100;
		SimulationExposure->Settings.AutoExposureMaxBrightness = Lighting.ExposureEV100;
	}
	const int32 CurrentPhase = static_cast<int32>(Lighting.Phase);
	if (LastLoggedDayPhase != CurrentPhase)
	{
		if (LastLoggedDayPhase == INDEX_NONE)
		{
			int32 DirectionalLightCount = 0;
			for (TActorIterator<ADirectionalLight> Iterator(GetWorld()); Iterator; ++Iterator)
			{
				++DirectionalLightCount;
				const ULightComponent* LightComponent = Iterator->GetLightComponent();
				UE_LOG(LogUEGT1, Display, TEXT("Day/night directional discovered: Name=%s Intensity=%.1f Rotation=%s"),
					*Iterator->GetName(), LightComponent ? LightComponent->Intensity : -1.0f,
					*Iterator->GetActorRotation().ToCompactString());
			}
			int32 SkyLightCount = 0;
			for (TActorIterator<ASkyLight> Iterator(GetWorld()); Iterator; ++Iterator)
			{
				++SkyLightCount;
				const USkyLightComponent* LightComponent = Iterator->GetLightComponent();
				UE_LOG(LogUEGT1, Display, TEXT("Day/night skylight discovered: Name=%s Intensity=%.3f"),
					*Iterator->GetName(), LightComponent ? LightComponent->Intensity : -1.0f);
			}
			UE_LOG(LogUEGT1, Display, TEXT("Day/night lighting ownership: DirectionalLights=%d SkyLights=%d Sun=%s Sky=%s Fog=%s Exposure=%s"),
				DirectionalLightCount, SkyLightCount, *GetNameSafe(SimulationSun.Get()), *GetNameSafe(SimulationSkyLight.Get()),
				*GetNameSafe(SimulationFog.Get()), *GetNameSafe(SimulationExposure.Get()));
		}
		LastLoggedDayPhase = CurrentPhase;
		UE_LOG(LogUEGT1, Display, TEXT("Day/night phase: %s Hour=%.2f SunElevation=%.1f SunAzimuth=%.1f SunLux=%.0f Sky=%.3f EV100=%.2f"),
			UEGT1DayNight::LexToString(Lighting.Phase), Lighting.Hour, Lighting.SunElevationDegrees,
			Lighting.SunAzimuthDegrees, Lighting.SunIntensityLux, Lighting.SkyLightIntensity, Lighting.ExposureEV100);
	}
}

FString UUEGT1TownSimulationSubsystem::ResolveSaveSlot(const FString& RequestedSlot) const
{
	return RequestedSlot.IsEmpty() ? UUEGT1TownSimulationSettings::Get().DefaultSaveSlot : RequestedSlot;
}
