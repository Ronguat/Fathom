#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FMGameMode.generated.h"

/** The harness game mode: the player controller, pawn and game state classes, and the ship spawned at BeginPlay. */
UCLASS()
class FATHOM_API AFMGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFMGameMode();

protected:
	virtual void BeginPlay() override;
};
