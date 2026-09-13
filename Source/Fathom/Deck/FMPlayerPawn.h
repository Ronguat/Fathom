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
 * for everyone else in the engine's default material, the weapon in each one's hand at the hand
 * socket, each posed from the combat state at the presented frame and otherwise from the idle
 * and walk clips by time. Input is the controller's key table, read when the input command is
 * authored; the attack's side is the last turn of the control yaw. On the owning client the view
 * turns with the deck's yaw at each finalized frame, so a player standing on a turning ship keeps
 * facing the same part of it. Writes INPUT on every key edge, POSE every PoseEveryFrames frames,
 * and BLADE on every release frame: the drawn weapons' tracers against the baked ones.
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

	/** Queues a station call for the next input command, one per command: wheel, sail_length, sail_angle, anchor, ladder. Every world applies it at that command's frame, this client as a prediction. */
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

	/** How fast a simulated proxy's drawn position closes the gap a base change opens, in cm per second; a gap wider than ProxySnapMax is a teleport and snaps. */
	UPROPERTY(EditAnywhere, Category="Fathom|Look")
	float ProxySnapDecay = 300.0f;

	UPROPERTY(EditAnywhere, Category="Fathom|Look")
	float ProxySnapMax = 400.0f;

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

	/** Draws a simulated proxy at its state's place plus the gap its last base change opened, the gap closing at ProxySnapDecay: a proxy is drawn on the deck as it is now but in world space at its own past frame, and the ship's travel between the two would show as a snap. */
	void DrawProxy(const FVector& Placed, bool bBased);

	/** Moves the visual root between the last two simulated frames by the framework's fraction, in base space when based, so a render frame between steps draws the pawn where the deck's presentation puts it. */
	void SmoothVisual();

	/** The base's transform as drawn this frame: a ship's mesh between its last two frames, any other base as it is. */
	static FTransform PresentedBase(const UPrimitiveComponent& Base);

	/** Poses one mesh from the combat state at the presented frame, or from the walk and idle clips by time, and evaluates it at once where its bones are read, so the frame draws the pose it was given; the body's pose remembers the attack frame it stands at for TraceBlade. */
	void Pose(USkeletalMeshComponent* Target, bool bFirstPerson);

	/** Reads the drawn weapons' tracers, the body's and for the owner the arms', against the baked tracers at the attack frame the body was just posed at, and writes BLADE. */
	void TraceBlade();

	/** The smoothed root Mover offsets between frames; the meshes and the camera ride it. */
	UPROPERTY(VisibleAnywhere, Category="Fathom")
	TObjectPtr<USceneComponent> Visual;

	UPROPERTY(VisibleAnywhere, Category="Fathom")
	TObjectPtr<UCapsuleComponent> Capsule;

	UPROPERTY(VisibleAnywhere, Category="Fathom")
	TObjectPtr<USkeletalMeshComponent> ArmsMesh;

	UPROPERTY(VisibleAnywhere, Category="Fathom")
	TObjectPtr<USkeletalMeshComponent> BodyMesh;

	/** The weapon in the arms' hand, seen by the owner, and in the body's, seen by everyone else. */
	UPROPERTY(VisibleAnywhere, Category="Fathom")
	TObjectPtr<USkeletalMeshComponent> ArmsWeapon;

	UPROPERTY(VisibleAnywhere, Category="Fathom")
	TObjectPtr<USkeletalMeshComponent> BodyWeapon;

	UPROPERTY(VisibleAnywhere, Category="Fathom")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, Category="Fathom")
	TObjectPtr<UCharacterMoverComponent> Mover;

	UPROPERTY(VisibleAnywhere, Category="Fathom")
	TObjectPtr<UFMCombatComponent> Combat;

private:
	FString RoleName() const;

	UPROPERTY() TObjectPtr<UAnimSequence> FirstPersonIdle;
	UPROPERTY() TObjectPtr<UAnimSequence> ThirdPersonIdle;
	UPROPERTY() TObjectPtr<UAnimSequence> FirstPersonWalk;
	UPROPERTY() TObjectPtr<UAnimSequence> ThirdPersonWalk;

	FName HarnessRole;
	TWeakObjectPtr<AFMShip> Ship;
	TArray<TPair<FName, float>> PendingStations;
	TWeakObjectPtr<const UPrimitiveComponent> SmoothBase;
	FVector SmoothFrom = FVector::ZeroVector;
	FVector SmoothTo = FVector::ZeroVector;
	float SmoothYawFrom = 0.0f;
	float SmoothYawTo = 0.0f;
	int32 SmoothFrame = -1;
	float BaseYawSeen = 0.0f;
	bool bHasBaseYaw = false;
	FVector ProxyGap = FVector::ZeroVector;
	FVector ProxyDrawn = FVector::ZeroVector;
	bool bProxyDrawn = false;
	bool bProxyWasBased = false;
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
	float LoopSeconds = 0.0f;
	uint8 PosedAttack = 0;
	float PosedAttackFrame = -1.0f;
	bool bPosedRelease = false;
};
