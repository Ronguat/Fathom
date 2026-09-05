#pragma once

#include "CoreMinimal.h"
#include "MovementMode.h"
#include "MovementModeTransition.h"
#include "FMSwimMode.generated.h"

/**
 * Swimming at the ocean's surface: horizontal movement from input at a capped speed, the
 * vertical velocity a spring toward the surface height at the pawn's position and frame.
 */
UCLASS()
class FATHOM_API UFMSwimMode : public UBaseMovementMode
{
	GENERATED_BODY()

public:
	/** Capsule centre above the surface when floating, cm. */
	UPROPERTY(EditAnywhere, Category="Fathom|Swim")
	float FloatHeight = 40.0f;

	UPROPERTY(EditAnywhere, Category="Fathom|Swim")
	float SurfaceSpring = 4.0f;

	UPROPERTY(EditAnywhere, Category="Fathom|Swim")
	float MaxVerticalSpeed = 300.0f;

	UPROPERTY(EditAnywhere, Category="Fathom|Swim")
	float MaxSwimSpeed = 250.0f;

	UPROPERTY(EditAnywhere, Category="Fathom|Swim")
	float TurningRate = 360.0f;

protected:
	virtual void GenerateMove_Implementation(const FMoverSimContext& SimContext, const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const override;
	virtual void SimulationTick_Implementation(const FSimulationTickParams& Params, FMoverTickEndData& OutputState) override;
};

/** Into Swimming when the pawn sinks EnterDepth under the surface; out to Falling when it is ExitHeight above it. */
UCLASS()
class FATHOM_API UFMSwimTransition : public UBaseMovementModeTransition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category="Fathom|Swim")
	float EnterDepth = 40.0f;

	UPROPERTY(EditAnywhere, Category="Fathom|Swim")
	float ExitHeight = 120.0f;

protected:
	virtual FTransitionEvalResult Evaluate_Implementation(const FSimulationTickParams& Params) const override;
};
