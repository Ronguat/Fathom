#include "Core/FMPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Net/FMTrace.h"
#include "Ship/FMShip.h"

AFMPlayerController::AFMPlayerController()
{
	ActionKeys.Add(TEXT("move_forward"), EKeys::W);
	ActionKeys.Add(TEXT("move_back"), EKeys::S);
	ActionKeys.Add(TEXT("move_left"), EKeys::A);
	ActionKeys.Add(TEXT("move_right"), EKeys::D);
	ActionKeys.Add(TEXT("jump"), EKeys::SpaceBar);
	ActionKeys.Add(TEXT("mark"), EKeys::M);
}

void AFMPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() && GetNetMode() == NM_Client)
	{
		if (const UFMTraceSubsystem* TraceSubsystem = UFMTraceSubsystem::Get(this))
		{
			ServerHello(TraceSubsystem->GetWorldTag());
		}
	}

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

void AFMPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	const FKey* MarkKey = ActionKeys.Find(TEXT("mark"));
	if (MarkKey && WasInputKeyJustPressed(*MarkKey))
	{
		FM_TRACE(this, TEXT("MARK category=hotkey"));
	}
	for (const TPair<FName, float>& Drive : PendingDrives)
	{
		if (HasAuthority())
		{
			ServerDriveShip_Implementation(Drive.Key, Drive.Value);
		}
		else
		{
			ServerDriveShip(Drive.Key, Drive.Value);
		}
	}
	PendingDrives.Reset();
}

void AFMPlayerController::ServerHello_Implementation(const FString& WorldTag)
{
	ClientWorldTag = WorldTag;
}

void AFMPlayerController::DriveShip(FName Input, float Value)
{
	PendingDrives.Emplace(Input, Value);
}

void AFMPlayerController::ServerDriveShip_Implementation(FName Input, float Value)
{
	if (AFMShip* Ship = AFMShip::Find(GetWorld()))
	{
		Ship->Apply(Input, Value);
	}
}

void AFMPlayerController::ServerRelayTrace_Implementation(const TArray<FString>& Lines)
{
	if (UFMTraceSubsystem* TraceSubsystem = UFMTraceSubsystem::Get(this))
	{
		TraceSubsystem->Ingest(Lines);
	}
}
