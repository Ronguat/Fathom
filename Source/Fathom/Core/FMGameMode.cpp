#include "Core/FMGameMode.h"

#include "Core/FMPlayerController.h"

AFMGameMode::AFMGameMode()
{
	PlayerControllerClass = AFMPlayerController::StaticClass();
}
