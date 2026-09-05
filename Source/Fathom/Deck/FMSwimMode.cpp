#include "Deck/FMSwimMode.h"

#include "MoveLibrary/MovementRecord.h"
#include "MoveLibrary/MovementUtils.h"
#include "MoverComponent.h"
#include "MoverDataModelTypes.h"
#include "MoverSimulationTypes.h"
#include "Engine/World.h"
#include "Ocean/FMOceanSubsystem.h"

namespace
{
	float SurfaceAt(const UWorld* World, const FVector& Location, int32 Frame)
	{
		const UFMOceanSubsystem* Ocean = World ? World->GetSubsystem<UFMOceanSubsystem>() : nullptr;
		const float Plane = GetDefault<UFMOceanSettings>()->PlaneZ;
		return Plane + (Ocean ? Ocean->HeightAt(FVector2f(Location.X, Location.Y), Frame) : 0.0f);
	}
}

void UFMSwimMode::GenerateMove_Implementation(const FMoverSimContext& SimContext, const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const
{
	const FCharacterDefaultInputs* Inputs = StartState.InputCmd.InputCollection.FindDataByType<FCharacterDefaultInputs>();
	const FMoverDefaultSyncState* Sync = StartState.SyncState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();
	if (!Sync)
	{
		return;
	}
	const float Dt = TimeStep.StepMs * 0.001f;
	FVector Move = Inputs ? Inputs->GetMoveInput_WorldSpace() : FVector::ZeroVector;
	Move.Z = 0.0;
	Move = Move.GetClampedToMaxSize(1.0);

	const FVector Location = Sync->GetLocation_WorldSpace();
	const float Target = SurfaceAt(GetWorld(), Location, TimeStep.ServerFrame) + FloatHeight;
	FVector Velocity = Move * MaxSwimSpeed;
	Velocity.Z = FMath::Clamp((Target - Location.Z) * SurfaceSpring, -MaxVerticalSpeed, MaxVerticalSpeed);

	FRotator Intended = Sync->GetOrientation_WorldSpace();
	if (Inputs && !Inputs->OrientationIntent.IsNearlyZero())
	{
		Intended = Inputs->GetOrientationIntentDir_WorldSpace().ToOrientationRotator();
	}
	Intended.Pitch = 0.0;
	Intended.Roll = 0.0;

	OutProposedMove.LinearVelocity = Velocity;
	OutProposedMove.AngularVelocityDegrees = UMovementUtils::ComputeAngularVelocityDegrees(Sync->GetOrientation_WorldSpace(), Intended, Dt, TurningRate);
	OutProposedMove.bHasDirIntent = !Move.IsNearlyZero();
	OutProposedMove.DirectionIntent = Move.GetSafeNormal();
}

void UFMSwimMode::SimulationTick_Implementation(const FSimulationTickParams& Params, FMoverTickEndData& OutputState)
{
	USceneComponent* UpdatedComponent = Params.MovingComps.UpdatedComponent.Get();
	const FMoverDefaultSyncState* Start = Params.StartState.SyncState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();
	if (!UpdatedComponent || !Start)
	{
		return;
	}
	FMoverDefaultSyncState& Out = OutputState.SyncState.SyncStateCollection.FindOrAddMutableDataByType<FMoverDefaultSyncState>();
	const float Dt = Params.TimeStep.StepMs * 0.001f;
	const FProposedMove& Move = Params.ProposedMove;
	FMovementRecord Record;
	Record.SetDeltaSeconds(Dt);

	const FRotator TargetOrient = UMovementUtils::ApplyAngularVelocityToRotator(Start->GetOrientation_WorldSpace(), Move.AngularVelocityDegrees, Dt);
	const FQuat TargetQuat = TargetOrient.Quaternion();
	const FVector Delta = Move.LinearVelocity * Dt;
	FHitResult Hit(1.0f);
	UMovementUtils::TrySafeMoveUpdatedComponent(Params.MovingComps, Delta, TargetQuat, true, Hit, ETeleportType::None, Record);
	if (Hit.IsValidBlockingHit())
	{
		UMovementUtils::TryMoveToSlideAlongSurface(Params.MovingComps, Delta, 1.0f - Hit.Time, TargetQuat, Hit.Normal, Hit, true, Record);
	}
	Out.SetTransforms_WorldSpace(UpdatedComponent->GetComponentLocation(), UpdatedComponent->GetComponentRotation(),
		Record.GetRelevantVelocity(), Move.AngularVelocityDegrees, nullptr);
	Out.MoveDirectionIntent = Move.bHasDirIntent ? Move.DirectionIntent : FVector::ZeroVector;
}

FTransitionEvalResult UFMSwimTransition::Evaluate_Implementation(const FSimulationTickParams& Params) const
{
	const FMoverDefaultSyncState* Sync = Params.StartState.SyncState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();
	if (!Sync)
	{
		return FTransitionEvalResult::NoTransition;
	}
	const FVector Location = Sync->GetLocation_WorldSpace();
	const float Surface = SurfaceAt(GetWorld(), Location, Params.TimeStep.ServerFrame);
	const bool bSwimming = Params.StartState.SyncState.MovementMode == DefaultModeNames::Swimming;
	if (!bSwimming && Location.Z < Surface - EnterDepth)
	{
		return FTransitionEvalResult(DefaultModeNames::Swimming);
	}
	if (bSwimming && Location.Z > Surface + ExitHeight)
	{
		return FTransitionEvalResult(DefaultModeNames::Falling);
	}
	return FTransitionEvalResult::NoTransition;
}
