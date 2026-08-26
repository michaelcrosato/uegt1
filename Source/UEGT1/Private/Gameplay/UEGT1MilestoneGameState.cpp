#include "Gameplay/UEGT1MilestoneGameState.h"

#include "UEGT1LogChannels.h"

void AUEGT1MilestoneGameState::RegisterWaystone(FName WaystoneId)
{
	if (WaystoneId.IsNone() || RegisteredWaystones.Contains(WaystoneId))
	{
		return;
	}

	RegisteredWaystones.Add(WaystoneId);
	UE_LOG(LogUEGT1, Log, TEXT("Waystone registered: Id=%s Total=%d"), *WaystoneId.ToString(), RegisteredWaystones.Num());
	BroadcastProgress();
}

bool AUEGT1MilestoneGameState::ActivateWaystone(FName WaystoneId)
{
	if (!RegisteredWaystones.Contains(WaystoneId) || ActivatedWaystones.Contains(WaystoneId))
	{
		return false;
	}

	ActivatedWaystones.Add(WaystoneId);
	UE_LOG(LogUEGT1, Display, TEXT("Waystone activated: Id=%s Progress=%d/%d Complete=%s"),
		*WaystoneId.ToString(), ActivatedWaystones.Num(), RegisteredWaystones.Num(), IsMilestoneComplete() ? TEXT("true") : TEXT("false"));
	BroadcastProgress();
	return true;
}

void AUEGT1MilestoneGameState::BroadcastProgress()
{
	OnObjectiveProgress.Broadcast(GetActivatedCount(), GetTotalCount(), IsMilestoneComplete());
}
