#pragma once

#include "CoreMinimal.h"
#include "Combat/FMAttackData.h"
#include "Engine/DeveloperSettings.h"
#include "MoverTypes.h"
#include "FMCombatTypes.generated.h"

class USkeletalMesh;

UENUM(BlueprintType)
enum class EFMCombatPhase : uint8
{
	Idle,
	Windup,
	Release,
	Recovery,
	Parry
};

/** The combat knobs; Config/DefaultGame.ini is their home. Frames at the fixed rate, distances in cm, angles in degrees. BlueprintType so editor Python can read it. */
UCLASS(config=Game, defaultconfig, BlueprintType, meta=(DisplayName="Fathom Combat"))
class FATHOM_API UFMCombatSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(config, EditAnywhere, Category="Attacks") TArray<TSoftObjectPtr<UFMAttackData>> Attacks;

	/** An attack starts early by this fraction of the round trip, capped; fm.MeleeAdvanceFraction and fm.MeleeAdvanceCapMs override at or above 0. */
	UPROPERTY(config, EditAnywhere, Category="Advance") float AdvanceFraction = 0.0f;
	UPROPERTY(config, EditAnywhere, Category="Advance") float AdvanceCapMs = 0.0f;

	UPROPERTY(config, EditAnywhere, Category="Parry") int32 ParryFrames = 30;
	/** The full angle in front of the defender within which an attacker is parried. */
	UPROPERTY(config, EditAnywhere, Category="Parry") float ParryConeDegrees = 120.0f;

	UPROPERTY(config, EditAnywhere, Category="Bodies") float BodyRadius = 34.0f;
	UPROPERTY(config, EditAnywhere, Category="Bodies") float BodyHalfHeight = 88.0f;
	/** The head sphere's centre above the capsule's centre. */
	UPROPERTY(config, EditAnywhere, Category="Bodies") float HeadHeight = 62.0f;
	UPROPERTY(config, EditAnywhere, Category="Bodies") float HeadRadius = 15.0f;
	/** Frames of body history the server keeps per pawn. */
	UPROPERTY(config, EditAnywhere, Category="Bodies") int32 HistoryFrames = 256;

	/** The weapon in the hand, drawn on the arms and on the body at WeaponSocket, offset and turned by the mount; its blade_base and blade_tip sockets bound the tracers. */
	UPROPERTY(config, EditAnywhere, Category="Weapon") TSoftObjectPtr<USkeletalMesh> WeaponMesh;
	UPROPERTY(config, EditAnywhere, Category="Weapon") FName WeaponSocket = TEXT("weapon_rSocket");
	UPROPERTY(config, EditAnywhere, Category="Weapon") FVector WeaponOffset = FVector::ZeroVector;
	UPROPERTY(config, EditAnywhere, Category="Weapon") FRotator WeaponRotation = FRotator::ZeroRotator;
	UPROPERTY(config, EditAnywhere, Category="Weapon") FName BladeBaseSocket = TEXT("blade_base");
	UPROPERTY(config, EditAnywhere, Category="Weapon") FName BladeTipSocket = TEXT("blade_tip");
	/** Tracers spaced from blade_base to blade_tip, each swept from its last frame's position to this one's. */
	UPROPERTY(config, EditAnywhere, Category="Weapon") int32 TracerCount = 8;

	UPROPERTY(config, EditAnywhere, Category="Presentation") TSoftObjectPtr<USkeletalMesh> FirstPersonMesh;
	UPROPERTY(config, EditAnywhere, Category="Presentation") TSoftObjectPtr<USkeletalMesh> ThirdPersonMesh;
	UPROPERTY(config, EditAnywhere, Category="Presentation") TSoftObjectPtr<UAnimSequence> FirstPersonParryPose;
	UPROPERTY(config, EditAnywhere, Category="Presentation") TSoftObjectPtr<UAnimSequence> ThirdPersonParryPose;
	/** Looped by time when idle, and while moving faster than WalkSpeedMin. */
	UPROPERTY(config, EditAnywhere, Category="Presentation") TSoftObjectPtr<UAnimSequence> FirstPersonIdle;
	UPROPERTY(config, EditAnywhere, Category="Presentation") TSoftObjectPtr<UAnimSequence> ThirdPersonIdle;
	UPROPERTY(config, EditAnywhere, Category="Presentation") TSoftObjectPtr<UAnimSequence> FirstPersonWalk;
	UPROPERTY(config, EditAnywhere, Category="Presentation") TSoftObjectPtr<UAnimSequence> ThirdPersonWalk;
	UPROPERTY(config, EditAnywhere, Category="Presentation") float WalkSpeedMin = 20.0f;
	/** The camera in pawn space: the head socket of the reference pose, which the bake prints. */
	UPROPERTY(config, EditAnywhere, Category="Presentation") FVector EyeOffset = FVector(0.0, 0.0, 64.0);
	/** The meshes under the capsule: origin at the feet, facing +X. */
	UPROPERTY(config, EditAnywhere, Category="Presentation") FVector MeshOffset = FVector(0.0, 0.0, -88.0);
	UPROPERTY(config, EditAnywhere, Category="Presentation") float MeshYaw = -90.0f;
	/** Added to the arms' place under the capsule, so the first-person hands sit in the view; the arms' weapon rides along, its divergence measured. */
	UPROPERTY(config, EditAnywhere, Category="Presentation") FVector ArmsOffset = FVector::ZeroVector;

	/** The advance in frames for a round trip, from the settings or the console variables. */
	int32 AdvanceFramesFor(float RoundTripMs, float StepMs) const;
};

/** The combat presses of one frame, authored on the owning client. */
USTRUCT()
struct FATHOM_API FFMCombatInputs : public FMoverDataStructBase
{
	GENERATED_BODY()

	/** The attack pressed this frame, None otherwise. */
	UPROPERTY() EFMAttackType Attack = EFMAttackType::None;
	UPROPERTY() EFMAttackSide Side = EFMAttackSide::Right;
	UPROPERTY() bool bParry = false;
	UPROPERTY() bool bFeint = false;
	/** The server frame this client's simulated proxies were drawn from when the command was authored, -1 before interpolation begins, and the fraction toward the next frame in 255ths. */
	UPROPERTY() int32 RenderedFrame = -1;
	UPROPERTY() uint8 RenderedFraction = 0;
	/** The advance the client predicts with, its player state's; the server clamps it to its own. */
	UPROPERTY() uint8 AdvanceFrames = 0;

	virtual FMoverDataStructBase* Clone() const override;
	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override;
	virtual UScriptStruct* GetScriptStruct() const override { return StaticStruct(); }
	virtual void ToString(FAnsiStringBuilderBase& Out) const override;
	virtual bool ShouldReconcile(const FMoverDataStructBase& AuthorityState) const override;
	virtual void Interpolate(const FMoverDataStructBase& From, const FMoverDataStructBase& To, float Pct) override;
	virtual void Merge(const FMoverDataStructBase& From) override;
};

template<>
struct TStructOpsTypeTraits<FFMCombatInputs> : public TStructOpsTypeTraitsBase2<FFMCombatInputs>
{
	enum { WithNetSerializer = true, WithCopy = true };
};

/**
 * Combat state in the sync state: the attack under way with the frame it started, the parry's
 * start frame, and the bodies this attack has already met. Reconciles on the first three; the
 * tallies are the server's and ride along.
 */
USTRUCT()
struct FATHOM_API FFMCombatState : public FMoverDataStructBase
{
	GENERATED_BODY()

	/** 1 to 6, one more than the index into the settings' attack table; 0 when idle. */
	UPROPERTY() uint8 Attack = 0;
	UPROPERTY() int32 AttackStart = -1;
	UPROPERTY() int32 ParryStart = -1;
	/** One bit per body this attack has hit or been parried by, by player id modulo 16. */
	UPROPERTY() uint16 HitMask = 0;
	UPROPERTY() uint8 Hits = 0;
	UPROPERTY() uint8 Parried = 0;

	bool IsAttacking() const { return Attack != 0 && AttackStart >= 0; }
	void ClearAttack();

	virtual FMoverDataStructBase* Clone() const override;
	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override;
	virtual UScriptStruct* GetScriptStruct() const override { return StaticStruct(); }
	virtual void ToString(FAnsiStringBuilderBase& Out) const override;
	virtual bool ShouldReconcile(const FMoverDataStructBase& AuthorityState) const override;
	virtual void Interpolate(const FMoverDataStructBase& From, const FMoverDataStructBase& To, float Pct) override;
	virtual void Merge(const FMoverDataStructBase& From) override;
};

template<>
struct TStructOpsTypeTraits<FFMCombatState> : public TStructOpsTypeTraitsBase2<FFMCombatState>
{
	enum { WithNetSerializer = true, WithCopy = true };
};

/** The rules every world applies alike. */
struct FATHOM_API FFMCombatRules
{
	static EFMCombatPhase PhaseAt(const FFMCombatState& State, int32 Frame, const UFMAttackData* Attack, int32 ParryFrames);
	static const TCHAR* PhaseName(EFMCombatPhase Phase);
	/** Distance from segment A to segment B, with the closest point on A. */
	static float SegmentToSegment(const FVector& A0, const FVector& A1, const FVector& B0, const FVector& B1, FVector& OutOnA);
	static float SegmentToPoint(const FVector& A0, const FVector& A1, const FVector& P, FVector& OutOnA);
};
