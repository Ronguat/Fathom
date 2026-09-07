#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FMAttackData.generated.h"

class UAnimSequence;

UENUM(BlueprintType)
enum class EFMAttackType : uint8
{
	None,
	Overhead,
	Horizontal,
	Thrust
};

UENUM(BlueprintType)
enum class EFMAttackSide : uint8
{
	Left,
	Right
};

/**
 * One attack baked from its first-person clip by Tools/Editor/bake-attacks.py: the blade's base
 * and tip at every simulation frame in pawn space, +X forward with the origin at the capsule's
 * centre, and the frames at which release and recovery begin. BlueprintType so editor Python
 * can write it.
 */
UCLASS(BlueprintType)
class FATHOM_API UFMAttackData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category="Attack") EFMAttackType Type = EFMAttackType::None;
	UPROPERTY(EditAnywhere, Category="Attack") EFMAttackSide Side = EFMAttackSide::Right;
	UPROPERTY(EditAnywhere, Category="Attack") TObjectPtr<UAnimSequence> FirstPerson;
	UPROPERTY(EditAnywhere, Category="Attack") TObjectPtr<UAnimSequence> ThirdPerson;

	/** Release begins at WindupFrames, recovery at WindupFrames + ReleaseFrames, idle at TotalFrames. */
	UPROPERTY(EditAnywhere, Category="Attack") int32 WindupFrames = 0;
	UPROPERTY(EditAnywhere, Category="Attack") int32 ReleaseFrames = 0;
	UPROPERTY(EditAnywhere, Category="Attack") int32 TotalFrames = 0;

	/** Indexed by attack frame, pawn space. */
	UPROPERTY(EditAnywhere, Category="Blade") TArray<FVector> BladeBase;
	UPROPERTY(EditAnywhere, Category="Blade") TArray<FVector> BladeTip;
	UPROPERTY(EditAnywhere, Category="Blade") float BladeLength = 0.0f;
	/** The blade's direction in the weapon bone's frame. */
	UPROPERTY(EditAnywhere, Category="Blade") FVector BladeAxis = FVector::XAxisVector;

	/** The name COMBAT lines carry: overhead_l, horizontal_r, thrust_l. */
	FString AttackName() const;
	int32 FrameCount() const { return FMath::Min(BladeBase.Num(), BladeTip.Num()); }
	bool BladeAt(int32 Frame, FVector& OutBase, FVector& OutTip) const;
};
