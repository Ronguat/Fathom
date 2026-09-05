#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FMTimeTools.generated.h"

/**
 *  A scriptable fixed time step, which only the command line sets otherwise. With it on, one tick
 *  is exactly the requested delta whatever the machine renders at.
 *
 *  Requires TargetRules.bWithFixedTimeStepSupport, which defaults true. SetFixedTimeStep returns
 *  what the engine adopted, so a build without support reads back false rather than free-running
 *  silently.
 */
UCLASS()
class FATHOMEDITOR_API UFMTimeTools : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Sets the delta first, so the first fixed tick already uses it. Returns FApp::UseFixedTimeStep(). */
	UFUNCTION(BlueprintCallable, Category="Fathom|Time")
	static bool SetFixedTimeStep(bool bEnabled, float DeltaSeconds = 0.016666f);

	UFUNCTION(BlueprintCallable, Category="Fathom|Time")
	static bool IsFixedTimeStep();

	UFUNCTION(BlueprintCallable, Category="Fathom|Time")
	static float GetFixedDeltaTime();
};
