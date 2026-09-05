#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "FMGameState.generated.h"

/**
 * Carries the session's ocean: the sea-state scalar and the wind vector, set once on the server
 * from the ocean settings or the fm.SeaState and fm.WindAngle console variables, replicated once.
 * SeaState is negative until the server has set it.
 */
UCLASS()
class FATHOM_API AFMGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	UPROPERTY(Replicated)
	float SeaState = -1.0f;

	UPROPERTY(Replicated)
	FVector2D Wind = FVector2D(1.0, 0.0);

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
