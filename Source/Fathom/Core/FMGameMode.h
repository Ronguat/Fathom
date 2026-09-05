#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FMGameMode.generated.h"

/** The harness game mode. Sets the player controller class and nothing else. */
UCLASS()
class FATHOM_API AFMGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFMGameMode();
};
