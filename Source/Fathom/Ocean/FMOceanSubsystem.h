#pragma once

#include "CoreMinimal.h"
#include "Ocean/FMOcean.h"
#include "Subsystems/WorldSubsystem.h"
#include "FMOceanSubsystem.generated.h"

class AFMOceanActor;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UMaterialParameterCollection;
class UTextureRenderTarget2D;

/**
 * The ocean on one world. Reads the sea state and wind off the game state, feeds the material
 * parameter collection every tick with the frame's time, spawns the surface on a world that
 * renders, and every ProbeEveryFrames draws the probe material into a float target, reads it
 * back and writes the OCEAN line: the CPU displacement at four points, the GPU's at the same
 * points, and the GPU-against-CPU error over the grid. A dedicated server writes the CPU only.
 */
UCLASS()
class FATHOM_API UFMOceanSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static UFMOceanSubsystem* Get(const UObject* WorldContext);

	bool IsReady() const { return SeaState >= 0.0f; }
	float GetSeaState() const { return SeaState; }
	const FVector2f& GetWind() const { return Wind; }
	const TArray<FFMWaveComponent>& GetWaves() const { return Waves; }

	/** Seconds of wave time at a shared frame. */
	float TimeOfFrame(int32 Frame) const { return static_cast<float>(Frame) / FrameRate; }
	/** Seconds of wave time as drawn: one frame behind the shared frame plus the prediction framework's leftover fraction. */
	float PresentedTime(int32 Frame) const;

	FVector3f Displace(const FVector2f& XY, int32 Frame) const;
	float HeightAt(const FVector2f& XY, int32 Frame) const;

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
	void OnTickEnd(UWorld* World, ELevelTick TickType, float DeltaSeconds);
	void SyncFromGameState();
	void PushCollection(int32 Frame);
	void Probe(int32 Frame);
	/** Keeps the surface under the local pawn, snapped to the plane's grid step. */
	void Follow();
	/** Drops the horizon plane under the sea state's deepest trough by the settings' margin. */
	void PlaceHorizon();
	bool Renders() const;

	UPROPERTY()
	TObjectPtr<UMaterialParameterCollection> Collection;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> ProbeMaterial;

	UPROPERTY()
	TObjectPtr<UTextureRenderTarget2D> ProbeTarget;

	UPROPERTY()
	TObjectPtr<AFMOceanActor> Surface;

	/** The flat plane to the horizon, under the deepest trough the session's sea state can make. */
	UPROPERTY()
	TObjectPtr<AFMOceanActor> Horizon;

	TArray<FFMWaveComponent> Waves;
	FVector2f Wind = FVector2f(1.0f, 0.0f);
	float SeaState = -1.0f;
	float FrameRate = 60.0f;
	bool bParamsPushed = false;
	int32 LastProbeFrame = -1;
	FDelegateHandle TickHandle;
};
