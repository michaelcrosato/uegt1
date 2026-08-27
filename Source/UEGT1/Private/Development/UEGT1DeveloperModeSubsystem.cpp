#include "Development/UEGT1DeveloperModeSubsystem.h"

#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UEGT1LogChannels.h"

void UUEGT1DeveloperModeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	bEnabled = FParse::Param(FCommandLine::Get(), TEXT("UEGT1DevMode"));
	bFlightEnabled = bEnabled && FParse::Param(FCommandLine::Get(), TEXT("UEGT1DevFlight"));
	if (bEnabled)
	{
		UE_LOG(LogUEGT1, Display, TEXT("Developer mode initialized: Invincible=true FastTravel=true Flight=%s"),
			bFlightEnabled ? TEXT("true") : TEXT("false"));
	}
}

void UUEGT1DeveloperModeSubsystem::SetEnabled(bool bInEnabled)
{
	bEnabled = bInEnabled;
	if (!bEnabled)
	{
		bFlightEnabled = false;
	}
	UE_LOG(LogUEGT1, Display, TEXT("Developer mode changed: Enabled=%s Invincible=%s FastTravel=%s Flight=%s"),
		bEnabled ? TEXT("true") : TEXT("false"), bEnabled ? TEXT("true") : TEXT("false"),
		bEnabled ? TEXT("true") : TEXT("false"), IsFlightEnabled() ? TEXT("true") : TEXT("false"));
}

void UUEGT1DeveloperModeSubsystem::SetFlightEnabled(bool bInFlightEnabled)
{
	if (bInFlightEnabled)
	{
		bEnabled = true;
	}
	bFlightEnabled = bEnabled && bInFlightEnabled;
	UE_LOG(LogUEGT1, Display, TEXT("Developer flight changed: Enabled=%s"), IsFlightEnabled() ? TEXT("true") : TEXT("false"));
}
