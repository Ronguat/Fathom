#pragma once

#include "CoreMinimal.h"
#include "DefaultMovementSet/Modes/FallingMode.h"
#include "DefaultMovementSet/Modes/WalkingMode.h"
#include "FMDeckModes.generated.h"

struct FMoverDefaultSyncState;
class UMoverComponent;
class UPrimitiveComponent;

/** Whether a base-space point lies over a ship's hull, with the settings' margin; false for any base that is not a ship's hull. */
bool FMDeckOverHull(const UPrimitiveComponent* Base, const FVector& BaseSpaceLocation);

/**
 * Keeps a ship's hull as the pawn's base through a tick whose mode would have dropped it, while
 * the pawn is still over the hull: the base carries the pawn as when walking, and the pawn's own
 * velocity stays relative to the deck. Past the hull's extent the base is dropped and the hull's
 * velocity imparted, which is the base change.
 */
void FMDeckKeepBase(UMoverComponent* Mover, const FMoverDefaultSyncState& Start, FMoverDefaultSyncState& Output);

/** Walking on the deck: a jump or a step into the air keeps the hull as the base while the pawn is over it. Stands in for the character mover's default walking mode through the pawn's object initializer, so the mover is its outer. */
UCLASS()
class FATHOM_API UFMDeckWalkingMode : public UWalkingMode
{
	GENERATED_BODY()

public:
	virtual void SimulationTick_Implementation(const FSimulationTickParams& Params, FMoverTickEndData& OutputState) override;
};

/**
 * Falling with the hull as the base: a pawn that left the deck with the base keeps it while over
 * the hull, integrating its fall in the deck's frame, and lands by the floor check as any fall
 * does. A pawn with no base falls in world space and takes the deck only by touching it.
 */
UCLASS()
class FATHOM_API UFMDeckFallingMode : public UFallingMode
{
	GENERATED_BODY()

public:
	virtual void SimulationTick_Implementation(const FSimulationTickParams& Params, FMoverTickEndData& OutputState) override;
};
