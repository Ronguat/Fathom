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
 * One attack baked from its third-person clip by Tools/Editor/bake-attacks.py: every tracer at
 * every simulation frame in pawn space, +X forward with the origin at the capsule's centre, the
 * tracers spaced from the weapon's blade_base socket to its blade_tip as the weapon rides the hand
 * socket, and the frames at which release and recovery begin. BlueprintType so editor Python can
 * write it.
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

	/** Frame-major: Tracers[Frame * TracerCount + Index], the first tracer at blade_base and the last at blade_tip. */
	UPROPERTY(EditAnywhere, Category="Blade") int32 TracerCount = 0;
	UPROPERTY(EditAnywhere, Category="Blade") TArray<FVector> Tracers;
	/** Socket to socket on the weapon. */
	UPROPERTY(EditAnywhere, Category="Blade") float BladeLength = 0.0f;

	/** The name COMBAT lines carry: overhead_l, horizontal_r, thrust_l. */
	FString AttackName() const;
	int32 FrameCount() const { return TracerCount > 0 ? Tracers.Num() / TracerCount : 0; }
	/** Every tracer at an attack frame; false when that frame is not baked. */
	bool TracersAt(int32 Frame, TConstArrayView<FVector>& Out) const;
	/** Every tracer between two attack frames by the fraction of Frame, the last baked frame past the end. */
	bool TracersBetween(float Frame, TArray<FVector>& Out) const;
};
