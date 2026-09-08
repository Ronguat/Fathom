#include "Deck/FMDeckModes.h"

#include "Components/PrimitiveComponent.h"
#include "MoveLibrary/BasedMovementUtils.h"
#include "MoverComponent.h"
#include "MoverDataModelTypes.h"
#include "MoverSimulationTypes.h"
#include "Ship/FMShip.h"

bool FMDeckOverHull(const UPrimitiveComponent* Base, const FVector& BaseSpaceLocation)
{
	if (!Base || !Cast<AFMShip>(Base->GetOwner()))
	{
		return false;
	}
	const UFMShipSettings* K = GetDefault<UFMShipSettings>();
	return FMath::Abs(BaseSpaceLocation.X) <= K->HalfLength + K->DeckMargin
		&& FMath::Abs(BaseSpaceLocation.Y) <= K->HalfWidth + K->DeckMargin;
}

void FMDeckKeepBase(UMoverComponent* Mover, const FMoverDefaultSyncState& Start, FMoverDefaultSyncState& Output)
{
	UPrimitiveComponent* Base = Start.GetMovementBase();
	if (!Base || Output.GetMovementBase())
	{
		return;
	}
	const FVector Location = Output.GetLocation_WorldSpace();
	const FRotator Orientation = Output.GetOrientation_WorldSpace();
	const FVector Velocity = Output.GetVelocity_WorldSpace();
	const FVector InBase = Base->GetComponentTransform().InverseTransformPositionNoScale(Location);
	if (!FMDeckOverHull(Base, InBase))
	{
		Output.SetTransforms_WorldSpace(Location, Orientation, Velocity + Base->GetComponentVelocity(), FVector::ZeroVector, nullptr);
		return;
	}
	Output.SetTransforms_WorldSpace(Location, Orientation, Velocity, FVector::ZeroVector, Base, Start.GetMovementBaseBoneName());
	FRelativeBaseInfo Info;
	Info.MovementBase = Base;
	Info.BoneName = Start.GetMovementBaseBoneName();
	Info.Location = Base->GetComponentLocation();
	Info.Rotation = Base->GetComponentQuat();
	Info.ContactLocalPosition = InBase;
	Info.WorldspaceOffsetFromContactPos = FVector::ZeroVector;
	if (UMoverBlackboard* Blackboard = Mover->GetSimBlackboard_Mutable())
	{
		Blackboard->Set(CommonBlackboard::LastFoundDynamicMovementBase, Info);
	}
}

void UFMDeckWalkingMode::SimulationTick_Implementation(const FSimulationTickParams& Params, FMoverTickEndData& OutputState)
{
	const FMoverDefaultSyncState* Start = Params.StartState.SyncState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();
	const FMoverDefaultSyncState StartCopy = Start ? *Start : FMoverDefaultSyncState();
	Super::SimulationTick_Implementation(Params, OutputState);
	if (FMoverDefaultSyncState* Output = OutputState.SyncState.SyncStateCollection.FindMutableDataByType<FMoverDefaultSyncState>())
	{
		FMDeckKeepBase(GetMoverComponent(), StartCopy, *Output);
	}
}

void UFMDeckFallingMode::SimulationTick_Implementation(const FSimulationTickParams& Params, FMoverTickEndData& OutputState)
{
	const FMoverDefaultSyncState* Start = Params.StartState.SyncState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();
	const FMoverDefaultSyncState StartCopy = Start ? *Start : FMoverDefaultSyncState();
	UMoverComponent* Mover = GetMoverComponent();
	UMoverBlackboard* Blackboard = Mover ? Mover->GetSimBlackboard_Mutable() : nullptr;
	FRelativeBaseInfo Carried;
	if (Start && Start->GetMovementBase() && Blackboard && Blackboard->TryGet(CommonBlackboard::LastFoundDynamicMovementBase, Carried) && Carried.HasRelativeInfo())
	{
		Carried = UBasedMovementUtils::ApplyMovementBaseChange(Mover, Carried, false);
		Blackboard->Set(CommonBlackboard::LastFoundDynamicMovementBase, Carried);
	}
	Super::SimulationTick_Implementation(Params, OutputState);
	if (FMoverDefaultSyncState* Output = OutputState.SyncState.SyncStateCollection.FindMutableDataByType<FMoverDefaultSyncState>())
	{
		FMDeckKeepBase(GetMoverComponent(), StartCopy, *Output);
	}
}
