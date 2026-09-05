#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FMGameMode.generated.h"

/** The harness game mode: the player controller and pawn classes, nothing else. */
UCLASS()
class FATHOM_API AFMGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFMGameMode();
};
