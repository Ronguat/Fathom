#pragma once

#include "CoreMinimal.h"
#include "Combat/FMCombatTypes.h"
#include "Components/ActorComponent.h"
#include "MoverSimulationTypes.h"
#include "FMCombatComponent.generated.h"

class UAnimSequence;
class UMoverComponent;
class UPrimitiveComponent;
struct FMoverDefaultSyncState;

/** One pawn's body at one frame: its base's space when based, world space otherwise. */
struct FFMBodySample
{
	int32 Frame = -1;
	FVector Location = FVector::ZeroVector;
	float Yaw = 0.0f;
	TWeakObjectPtr<const UPrimitiveComponent> Base;
	int32 ParryStart = -1;
	uint8 Attack = 0;
	int32 AttackStart = -1;
};

/**
 * Combat on a Mover pawn. Reads FFMCombatInputs from each frame's command and writes
 * FFMCombatState into the sync state inside the simulation tick, so every world runs the same
 * transitions and a mistaken prediction rolls back. The server keeps every frame's body, sweeps
 * the blade during release against the other bodies at the frame the attacker's command says it
 * rendered, and tallies into replicated counts; once a second it refreshes the player state's
 * advance from the connection's round trip. Writes COMBAT on every world at a phase change,
 * HIT, PARRY and SWING on the server, SCORE wherever the tallies change.
 */
UCLASS()
class FATHOM_API UFMCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFMCombatComponent();

	UPROPERTY(ReplicatedUsing=OnRep_Score) int32 HitsTaken = 0;
	UPROPERTY(ReplicatedUsing=OnRep_Score) int32 HitsDealt = 0;
	UPROPERTY(ReplicatedUsing=OnRep_Score) int32 ParriesMade = 0;

	/** The table entry for a state's Attack; null when idle or not loaded. */
	const UFMAttackData* AttackData(uint8 Attack) const;
	/** 1 to 6 for a type and side, 0 for none. */
	uint8 AttackIndex(EFMAttackType Type, EFMAttackSide Side) const;
	const FFMCombatState* State() const;
	EFMCombatPhase PhaseAt(const FFMCombatState& S, int32 Frame) const;
	/** The body at a frame from the server's history; false when that frame is not kept. */
	bool BodyAt(int32 Frame, FFMBodySample& Out) const;
	/** The body between a frame and the next by a fraction, as a client draws its proxies; the frame alone when the next is not kept. */
	bool BodyBetween(int32 Frame, float Fraction, FFMBodySample& Out) const;
	bool Latest(FFMBodySample& Out) const;

	/** The server frame this client's simulated proxies are drawn from, -1 before interpolation begins, and the fraction toward the next. */
	static int32 RenderedFrame(const UWorld* World, float& OutFraction);
	/** The frame the owner is drawn at, with the fraction between steps. */
	double PresentedFrame() const;
	/** The clip and the time into it at the presented frame; false when idle. */
	bool Presented(bool bFirstPerson, const UAnimSequence*& OutClip, float& OutTime) const;

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UFUNCTION()
	void HandlePreSimulationTick(const FMoverTimeStep& TimeStep, const FMoverInputCmdContext& InputCmd);

	UFUNCTION()
	void HandlePostMovement(const FMoverTimeStep& TimeStep, FMoverSyncState& SyncState, FMoverAuxStateContext& AuxState);

	UFUNCTION()
	void HandlePostFinalize(const FMoverSyncState& SyncState, const FMoverAuxStateContext& AuxState);

	UFUNCTION()
	void OnRep_Score();

private:
	void Transition(FFMCombatState& S, const FFMCombatInputs& In, int32 Frame);
	void Record(const FMoverDefaultSyncState& Body, const FFMCombatState& S, int32 Frame);
	void Sweep(FFMCombatState& S, const FMoverDefaultSyncState& Body, int32 Frame, int32 AtFrame, float AtFraction);
	void RefreshAdvance(float StepMs);
	int32 PlayerId() const;
	void TraceScore();

	UPROPERTY() TArray<TObjectPtr<UFMAttackData>> Attacks;
	UPROPERTY() TObjectPtr<UMoverComponent> Mover;
	UPROPERTY() TObjectPtr<UAnimSequence> FirstPersonParry;
	UPROPERTY() TObjectPtr<UAnimSequence> ThirdPersonParry;

	TArray<FFMBodySample> History;
	int32 LatestFrame = -1;
	FFMCombatInputs FrameInputs;
	int32 FrameInputsFrame = -1;
	EFMCombatPhase TracedPhase = EFMCombatPhase::Idle;
	uint8 TracedAttack = 0;
	int32 TracedStart = -1;
	int32 TracedParry = -1;
};
