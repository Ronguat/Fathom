#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "FMOcean.generated.h"

class UMaterialInterface;
class UMaterialParameterCollection;

/** One Gerstner component: wavelength in cm, amplitude and steepness at sea state 1, direction offset from the wind in degrees. */
USTRUCT()
struct FFMWaveComponent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Wave")
	float Wavelength = 4000.0f;

	UPROPERTY(EditAnywhere, Category="Wave")
	float Amplitude = 60.0f;

	UPROPERTY(EditAnywhere, Category="Wave")
	float Steepness = 0.6f;

	UPROPERTY(EditAnywhere, Category="Wave")
	float AngleDegrees = 0.0f;
};

/** The ocean's knobs: the components, the session defaults, the assets, the probe and the plane. Config/DefaultGame.ini is their home. */
UCLASS(config=Game, defaultconfig, meta=(DisplayName="Fathom Ocean"))
class FATHOM_API UFMOceanSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(config, EditAnywhere, Category="Waves")
	TArray<FFMWaveComponent> Components;

	UPROPERTY(config, EditAnywhere, Category="Session", meta=(ClampMin="0"))
	float SeaState = 0.5f;

	/** The largest sea state a session takes: amplitude and steepness both scale with it, so the summed steepness loops the surface past the square root of one over the components' summed steepness times amplitude times wavenumber. */
	UPROPERTY(config, EditAnywhere, Category="Session", meta=(ClampMin="0"))
	float SeaStateMax = 2.4f;

	UPROPERTY(config, EditAnywhere, Category="Session")
	float WindAngleDegrees = 0.0f;

	UPROPERTY(config, EditAnywhere, Category="Assets")
	TSoftObjectPtr<UMaterialParameterCollection> Collection;

	UPROPERTY(config, EditAnywhere, Category="Assets")
	TSoftObjectPtr<UMaterialInterface> SurfaceMaterial;

	UPROPERTY(config, EditAnywhere, Category="Assets")
	TSoftObjectPtr<UMaterialInterface> ProbeMaterial;

	UPROPERTY(config, EditAnywhere, Category="Probe")
	int32 ProbeEveryFrames = 60;

	UPROPERTY(config, EditAnywhere, Category="Probe")
	int32 ProbeCells = 16;

	UPROPERTY(config, EditAnywhere, Category="Probe")
	float ProbeExtent = 8000.0f;

	UPROPERTY(config, EditAnywhere, Category="Plane")
	float PlaneSize = 40000.0f;

	UPROPERTY(config, EditAnywhere, Category="Plane")
	int32 PlaneSteps = 200;

	UPROPERTY(config, EditAnywhere, Category="Plane")
	float PlaneZ = -200.0f;

	/** A flat plane this wide under the displaced one, reaching the horizon. */
	UPROPERTY(config, EditAnywhere, Category="Plane")
	float HorizonSize = 4000000.0f;

	/** How far the horizon plane sits under the deepest trough the session's sea state can make. */
	UPROPERTY(config, EditAnywhere, Category="Plane")
	float HorizonMargin = 20.0f;
};

/** The wave function in single floats, the same formulation as Shaders/FMOcean.ush. */
namespace FMOcean
{
	constexpr float Pi = 3.14159265358979f;
	constexpr float Gravity = 980.0f;
	constexpr float ProbeOffset = 5000.0f;

	FATHOM_API FVector3f Wave(const FVector2f& P, float Time, float SeaState, const FVector2f& Wind, const FFMWaveComponent& W);
	FATHOM_API FVector3f Displace(const FVector2f& P, float Time, float SeaState, const FVector2f& Wind, const TArray<FFMWaveComponent>& Waves);

	/** The source point whose displaced position lands on P, by fixed-point iteration. */
	FATHOM_API FVector2f SourceOf(const FVector2f& P, float Time, float SeaState, const FVector2f& Wind, const TArray<FFMWaveComponent>& Waves, int32 Iterations = 3);

	/** The surface height over P: the displacement's height at SourceOf(P). */
	FATHOM_API float HeightAt(const FVector2f& P, float Time, float SeaState, const FVector2f& Wind, const TArray<FFMWaveComponent>& Waves, int32 Iterations = 3);

	/** How far SourceOf(P)'s displaced position misses P, in cm. */
	FATHOM_API float InversionResidual(const FVector2f& P, float Time, float SeaState, const FVector2f& Wind, const TArray<FFMWaveComponent>& Waves, int32 Iterations = 3);

	/** A unit vector for an angle in degrees, X along zero. */
	FATHOM_API FVector2f WindFromAngle(float Degrees);
}
