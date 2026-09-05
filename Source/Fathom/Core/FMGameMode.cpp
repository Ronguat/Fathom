#include "Core/FMGameMode.h"

#include "Core/FMGameState.h"
#include "Core/FMPlayerController.h"
#include "Deck/FMPlayerPawn.h"
#include "Ship/FMShip.h"

AFMGameMode::AFMGameMode()
{
	PlayerControllerClass = AFMPlayerController::StaticClass();
	DefaultPawnClass = AFMPlayerPawn::StaticClass();
	GameStateClass = AFMGameState::StaticClass();
}

void AFMGameMode::BeginPlay()
{
	Super::BeginPlay();
	const UFMShipSettings* K = GetDefault<UFMShipSettings>();
	GetWorld()->SpawnActor<AFMShip>(FVector(K->SpawnXY.X, K->SpawnXY.Y, 0.0), FRotator(0.0f, K->SpawnHeading, 0.0f));
}
