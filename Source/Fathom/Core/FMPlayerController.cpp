#include "Core/FMPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"

void AFMPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		LocalPlayer ? LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>() : nullptr;
	if (!Subsystem)
	{
		return;
	}
	int32 Priority = 0;
	for (UInputMappingContext* Context : DefaultMappingContexts)
	{
		if (Context)
		{
			Subsystem->AddMappingContext(Context, Priority++);
		}
	}
}
