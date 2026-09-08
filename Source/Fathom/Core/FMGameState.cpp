#include "Core/FMGameState.h"

#include "HAL/IConsoleManager.h"
#include "Net/UnrealNetwork.h"
#include "Ocean/FMOcean.h"

namespace
{
	TAutoConsoleVariable<float> CVarSeaState(TEXT("fm.SeaState"), -1.0f,
		TEXT("The session's sea state, 0 to the ocean settings' SeaStateMax, read by the server at start; below 0 reads the ocean settings"));
	TAutoConsoleVariable<float> CVarWindAngle(TEXT("fm.WindAngle"), -1000.0f,
		TEXT("The session's wind angle in degrees, read by the server at start; below -999 reads the ocean settings"));
}

void AFMGameState::BeginPlay()
{
	Super::BeginPlay();
	if (!HasAuthority())
	{
		return;
	}
	const UFMOceanSettings* Settings = GetDefault<UFMOceanSettings>();
	const float RequestedSeaState = CVarSeaState.GetValueOnGameThread();
	const float RequestedAngle = CVarWindAngle.GetValueOnGameThread();
	SeaState = FMath::Clamp(RequestedSeaState >= 0.0f ? RequestedSeaState : Settings->SeaState, 0.0f, FMath::Max(0.0f, Settings->SeaStateMax));
	const float Angle = RequestedAngle > -999.0f ? RequestedAngle : Settings->WindAngleDegrees;
	const FVector2f Unit = FMOcean::WindFromAngle(Angle);
	Wind = FVector2D(Unit.X, Unit.Y);
}

void AFMGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFMGameState, SeaState);
	DOREPLIFETIME(AFMGameState, Wind);
}
