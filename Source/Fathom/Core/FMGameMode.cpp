#include "Core/FMGameMode.h"

#include "Core/FMGameState.h"
#include "Core/FMPlayerController.h"
#include "Deck/FMPlayerPawn.h"

AFMGameMode::AFMGameMode()
{
	PlayerControllerClass = AFMPlayerController::StaticClass();
	DefaultPawnClass = AFMPlayerPawn::StaticClass();
	GameStateClass = AFMGameState::StaticClass();
}
