#include "Core/FMGameMode.h"

#include "Core/FMPlayerController.h"
#include "Deck/FMPlayerPawn.h"

AFMGameMode::AFMGameMode()
{
	PlayerControllerClass = AFMPlayerController::StaticClass();
	DefaultPawnClass = AFMPlayerPawn::StaticClass();
}
