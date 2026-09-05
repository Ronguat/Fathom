#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FMPlayerController.generated.h"

class UInputMappingContext;

/** Adds DefaultMappingContexts to the local player Enhanced Input subsystem at BeginPlay, in array order. */
UCLASS()
class FATHOM_API AFMPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category="Input")
	TArray<TObjectPtr<UInputMappingContext>> DefaultMappingContexts;

	virtual void BeginPlay() override;
};
