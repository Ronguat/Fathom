#include "Core/FMGameState.h"

#include "Engine/NetConnection.h"
#include "Engine/NetDriver.h"
#include "HAL/IConsoleManager.h"
#include "Net/FMTrace.h"
#include "Net/UnrealNetwork.h"
#include "NetworkPredictionWorldManager.h"
#include "Ocean/FMOcean.h"
#include "Ship/FMShip.h"

namespace
{
	TAutoConsoleVariable<float> CVarSeaState(TEXT("fm.SeaState"), -1.0f,
		TEXT("The session's sea state, 0 to the ocean settings' SeaStateMax, read by the server at start; below 0 reads the ocean settings"));
	TAutoConsoleVariable<float> CVarWindAngle(TEXT("fm.WindAngle"), -1000.0f,
		TEXT("The session's wind angle in degrees, read by the server at start; below -999 reads the ocean settings"));
}

AFMGameState::AFMGameState()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AFMGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	const UNetDriver* Driver = GetNetDriver();
	if (!HasAuthority() || !Driver)
	{
		return;
	}
	int32 WorstRoundTripMs = 0;
	bool bUnmeasured = false;
	for (const TObjectPtr<UNetConnection>& Connection : Driver->ClientConnections)
	{
		if (!Connection)
		{
			continue;
		}
		bUnmeasured |= Connection->AvgLag <= 0.0f;
		WorstRoundTripMs = FMath::Max(WorstRoundTripMs, FMath::RoundToInt(Connection->AvgLag * 1000.0f));
	}
	const UNetworkPredictionWorldManager* Prediction = GetWorld()->GetSubsystem<UNetworkPredictionWorldManager>();
	const float StepMs = Prediction ? 1000.0f / FMath::Max(1, Prediction->GetSettings().FixedTickFrameRate) : 1000.0f / 60.0f;
	const UFMShipSettings* K = GetDefault<UFMShipSettings>();
	const int32 Frames = bUnmeasured ? K->StationDelayMaxFrames
		: FMath::Clamp(FMath::CeilToInt32(K->StationDelayFactor * WorstRoundTripMs / StepMs) + K->StationDelayBaseFrames, 0, K->StationDelayMaxFrames);
	if (!bDelaySet || Frames != StationDelayFrames)
	{
		FM_TRACE(this, TEXT("STATIONDELAY frames=%d worst_ms=%d clients=%d"), Frames, WorstRoundTripMs, Driver->ClientConnections.Num());
	}
	bDelaySet = true;
	StationDelayFrames = Frames;
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
	DOREPLIFETIME(AFMGameState, StationDelayFrames);
}
