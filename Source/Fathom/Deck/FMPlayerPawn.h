#pragma once

#include "CoreMinimal.h"
#include "Combat/FMAttackData.h"
#include "DefaultMovementSet/InstantMovementEffects/BasicInstantMovementEffects.h"
#include "GameFramework/Pawn.h"
#include "MoverSimulationTypes.h"
#include "FMPlayerPawn.generated.h"

struct FMoverDefaultSyncState;
class AFMShip;
class UAnimSequence;
class UPrimitiveComponent;

class UCameraComponent;
class UCapsuleComponent;
class UCharacterMoverComponent;
class UFMCombatComponent;
class USkeletalMeshComponent;

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
 * backend, first-person camera at the eye, the delivered arms drawn for the owner and the body
 * for everyone else in the engine's default material, each posed from the combat state at the
 * presented frame. Input is the
 * controller's key table, read when the input command is authored; the attack's side is the last
 * turn of the control yaw. On the owning client the view turns with the deck's yaw at each
 * finalized frame, so a player standing on a turning ship keeps facing the same part of it.
 * Writes INPUT on every key edge and POSE every PoseEveryFrames frames.
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

	/** The sync state's location in its base's space; false when it stands on no base. */
	bool ShipSpaceLocation(FVector& Out) const;
	FString MovementModeName() const;
	UFMCombatComponent* GetCombat() const { return Combat; }

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
	void HandlePreSimulationTick(const FMoverTimeStep& TimeStep, const FMoverInputCmdContext& InputCmd);

	UFUNCTION()
	void HandlePostFinalize(const FMoverSyncState& SyncState, const FMoverAuxStateContext& AuxState);

	UFUNCTION()
	void HandleRollback(const FMoverTimeStep& CurrentTimeStep, const FMoverTimeStep& ExpungedTimeStep);

	/** Places a simulated proxy where its base-space state stands on the base as this world presents it, turned by the base's yaw alone so the body stays upright as the deck rolls. */
	void PlaceOnBase(const FMoverDefaultSyncState& State, const UPrimitiveComponent& Base);

	/** Moves the visual root between the last two simulated frames by the framework's fraction, in base space when based, so a render frame between steps draws the pawn where the deck's presentation puts it. */
	void SmoothVisual();

	/** The base's transform as drawn this frame: a ship's mesh between its last two frames, any other base as it is. */
	static FTransform PresentedBase(const UPrimitiveComponent& Base);

	/** Poses one mesh from the combat state at the presented frame, the first attack's first frame when idle. */
	void Pose(USkeletalMeshComponent* Target, bool bFirstPerson);

	/** The smoothed root Mover offsets between frames; the meshes and the camera ride it. */
	UPROPERTY(VisibleAnywhere, Category="Fathom")
	TObjectPtr<USceneComponent> Visual;

	UPROPERTY(VisibleAnywhere, Category="Fathom")
	TObjectPtr<UCapsuleComponent> Capsule;

	UPROPERTY(VisibleAnywhere, Category="Fathom")
	TObjectPtr<USkeletalMeshComponent> ArmsMesh;

	UPROPERTY(VisibleAnywhere, Category="Fathom")
	TObjectPtr<USkeletalMeshComponent> BodyMesh;

	UPROPERTY(VisibleAnywhere, Category="Fathom")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, Category="Fathom")
	TObjectPtr<UCharacterMoverComponent> Mover;

	UPROPERTY(VisibleAnywhere, Category="Fathom")
	TObjectPtr<UFMCombatComponent> Combat;

private:
	FString RoleName() const;

	FName HarnessRole;
	TWeakObjectPtr<AFMShip> Ship;
	TWeakObjectPtr<const UPrimitiveComponent> SmoothBase;
	FVector SmoothFrom = FVector::ZeroVector;
	FVector SmoothTo = FVector::ZeroVector;
	float SmoothYawFrom = 0.0f;
	float SmoothYawTo = 0.0f;
	int32 SmoothFrame = -1;
	float BaseYawSeen = 0.0f;
	bool bHasBaseYaw = false;
	bool bFlying = false;
	bool bJumpWasDown = false;
	int32 Rollbacks = 0;
	int32 PendingRollbackTo = -1;
	int32 PendingRollbackFrom = -1;
	TMap<FName, bool> KeyWasDown;
	EFMAttackSide Side = EFMAttackSide::Right;
	float LastControlYaw = 0.0f;
	bool bHasLastControlYaw = false;
	TMap<USkeletalMeshComponent*, const UAnimSequence*> Posed;
};
