#include "Core/FMPlayerState.h"

#include "Net/UnrealNetwork.h"

void AFMPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFMPlayerState, AdvanceFrames);
	DOREPLIFETIME(AFMPlayerState, RoundTripMs);
}
