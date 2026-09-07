#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "FMPlayerState.generated.h"

/** Carries the advance the server grants this player's attacks, from its connection's round trip, refreshed once a second. */
UCLASS()
class FATHOM_API AFMPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	UPROPERTY(Replicated)
	int32 AdvanceFrames = 0;

	UPROPERTY(Replicated)
	int32 RoundTripMs = 0;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
