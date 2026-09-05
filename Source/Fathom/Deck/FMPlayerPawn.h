#pragma once

#include "CoreMinimal.h"
#include "DefaultMovementSet/InstantMovementEffects/BasicInstantMovementEffects.h"
#include "GameFramework/Pawn.h"
#include "MoverSimulationTypes.h"
#include "FMPlayerPawn.generated.h"

struct FMoverDefaultSyncState;
class UPrimitiveComponent;

class UCameraComponent;
class UCapsuleComponent;
class UCharacterMoverComponent;
class UStaticMeshComponent;

/** A teleport that lands in Falling, so a pawn dropped over a deck or a floor settles onto it. */
USTRUCT()
struct FFMTeleportEffect : public FTeleportEffect
{
	GENERATED_BODY()

	virtual bool ApplyMovementEffect(FApplyMovementEffectParams& ApplyEffectParams, FMoverSyncState& OutputState) override;
	virtual FInstantMovementEffect* Clone() const override;
	virtual UScriptStruct* GetScriptStruct() const override;
};

/**
 * The player's pawn: a capsule driven by a Character Mover component on the Network Prediction
 * backend, first-person camera. Input is the controller's key table, read when the input
 * command is authored. Writes INPUT on every key edge and POSE every PoseEveryFrames frames.
 */
UCLASS()
class FATHOM_API AFMPlayerPawn : public APawn, public IMoverInputProducerInterface
{
	GENERATED_BODY()

public:
	AFMPlayerPawn(const FObjectInitializer& ObjectInitializer);

	/** The name INPUT lines carry for this pawn; `pid<n>` until set. Local to the caller's world. */
	UFUNCTION(BlueprintCallable, Category="Fathom|Harness")
	void SetHarnessRole(FName InRole);

	/** Moves the pawn through the simulation at its next frame. Server only. */
	UFUNCTION(BlueprintCallable, Category="Fathom|Harness")
	void HarnessTeleport(FVector Location, float Yaw);

	/** Drives one of the ship's stations through the controller: wheel, sail_length, sail_angle, anchor. */
	UFUNCTION(BlueprintCallable, Category="Fathom|Ship")
	void DriveShip(FName Input, float Value);

	/** The sync state's location, which the POSE line reports. */
	UFUNCTION(BlueprintPure, Category="Fathom|Harness")
	FVector GetSimLocation() const;

	/** The simulation frame last finalized, in server numbering. */
	UFUNCTION(BlueprintPure, Category="Fathom|Harness")
	int32 GetSimFrame() const;

	UPROPERTY(EditAnywhere, Category="Fathom|Trace")
	int32 PoseEveryFrames = 6;

	UPROPERTY(EditAnywhere, Category="Fathom|Look")
	float LookScale = 0.5f;

protected:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void ProduceInput_Implementation(int32 SimTimeMs, FMoverInputCmdContext& InputCmdResult) override;

	UFUNCTION()
	void HandlePostFinalize(const FMoverSyncState& SyncState, const FMoverAuxStateContext& AuxState);

	UFUNCTION()
	void HandleRollback(const FMoverTimeStep& CurrentTimeStep, const FMoverTimeStep& ExpungedTimeStep);

	/** Places a simulated proxy where its base-space state stands on the base as this world presents it. */
	void PlaceOnBase(const FMoverDefaultSyncState& State, const UPrimitiveComponent& Base);

	/** The base's transform as drawn this frame: a ship's mesh between its last two frames, any other base as it is. */
	static FTransform PresentedBase(const UPrimitiveComponent& Base);

	/** The smoothed root Mover offsets between frames; the mesh and the camera ride it. */
	UPROPERTY(VisibleAnywhere, Category="Fathom")
	TObjectPtr<USceneComponent> Visual;

	UPROPERTY(VisibleAnywhere, Category="Fathom")
	TObjectPtr<UCapsuleComponent> Capsule;

	UPROPERTY(VisibleAnywhere, Category="Fathom")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, Category="Fathom")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, Category="Fathom")
	TObjectPtr<UCharacterMoverComponent> Mover;

private:
	FString RoleName() const;

	FName HarnessRole;
	bool bJumpWasDown = false;
	int32 Rollbacks = 0;
	int32 PendingRollbackTo = -1;
	int32 PendingRollbackFrom = -1;
	TMap<FName, bool> KeyWasDown;
};
