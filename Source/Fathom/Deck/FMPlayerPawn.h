#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MoverSimulationTypes.h"
#include "FMPlayerPawn.generated.h"

class UCameraComponent;
class UCapsuleComponent;
class UCharacterMoverComponent;
class UStaticMeshComponent;

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
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void ProduceInput_Implementation(int32 SimTimeMs, FMoverInputCmdContext& InputCmdResult) override;

	UFUNCTION()
	void HandlePostFinalize(const FMoverSyncState& SyncState, const FMoverAuxStateContext& AuxState);

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
	TMap<FName, bool> KeyWasDown;
};
