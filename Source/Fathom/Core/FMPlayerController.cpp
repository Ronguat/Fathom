#include "Core/FMPlayerController.h"

#include "Deck/FMPlayerPawn.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "InputKeyEventArgs.h"
#include "InputMappingContext.h"
#include "Net/FMTrace.h"
#include "Ship/FMShip.h"

namespace
{
	constexpr double NoticeSeconds = 2.0;
}

AFMPlayerController::AFMPlayerController()
{
	ActionKeys.Add(TEXT("move_forward"), EKeys::W);
	ActionKeys.Add(TEXT("move_back"), EKeys::S);
	ActionKeys.Add(TEXT("move_left"), EKeys::A);
	ActionKeys.Add(TEXT("move_right"), EKeys::D);
	ActionKeys.Add(TEXT("jump"), EKeys::SpaceBar);
	ActionKeys.Add(TEXT("mark"), EKeys::M);
	ActionKeys.Add(TEXT("attack_horizontal"), EKeys::LeftMouseButton);
	ActionKeys.Add(TEXT("attack_overhead"), EKeys::MouseScrollDown);
	ActionKeys.Add(TEXT("attack_thrust"), EKeys::MouseScrollUp);
	ActionKeys.Add(TEXT("parry"), EKeys::RightMouseButton);
	ActionKeys.Add(TEXT("feint"), EKeys::Q);
	ActionKeys.Add(TEXT("wheel_left"), EKeys::Left);
	ActionKeys.Add(TEXT("wheel_right"), EKeys::Right);
	ActionKeys.Add(TEXT("sail_up"), EKeys::Up);
	ActionKeys.Add(TEXT("sail_down"), EKeys::Down);
	ActionKeys.Add(TEXT("angle_left"), EKeys::LeftBracket);
	ActionKeys.Add(TEXT("angle_right"), EKeys::RightBracket);
	ActionKeys.Add(TEXT("anchor"), EKeys::X);
	ActionKeys.Add(TEXT("ladder"), EKeys::E);
	ActionKeys.Add(TEXT("board"), EKeys::B);
	ActionKeys.Add(TEXT("fly"), EKeys::F);
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

bool AFMPlayerController::IsMomentary(const FKey& Key)
{
	return Key == EKeys::MouseScrollUp || Key == EKeys::MouseScrollDown;
}

bool AFMPlayerController::InputKey(const FInputKeyEventArgs& Params)
{
	if (Params.Event == IE_Pressed || Params.Event == IE_Released)
	{
		for (const TPair<FName, FKey>& Binding : ActionKeys)
		{
			if (Binding.Value != Params.Key)
			{
				continue;
			}
			if (Params.Event == IE_Pressed)
			{
				Latched.Add(Binding.Key);
				EventDown.Add(Binding.Key);
			}
			else
			{
				EventDown.Remove(Binding.Key);
			}
		}
	}
	return Super::InputKey(Params);
}

void AFMPlayerController::TakeLatched(TSet<FName>& OutPressed)
{
	OutPressed = MoveTemp(Latched);
	Latched.Reset();
}

bool AFMPlayerController::IsActionDown(FName Action) const
{
	if (Latched.Contains(Action))
	{
		return true;
	}
	const FKey* Key = ActionKeys.Find(Action);
	return Key && EventDown.Contains(Action) && IsInputKeyDown(*Key);
}

void AFMPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	const FKey* MarkKey = ActionKeys.Find(TEXT("mark"));
	if (MarkKey && WasInputKeyJustPressed(*MarkKey))
	{
		FM_TRACE(this, TEXT("MARK category=hotkey"));
	}
	if (IsLocalController())
	{
		DriveFromKeys();
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

void AFMPlayerController::DriveFromKeys()
{
	const auto Pressed = [this](const TCHAR* Action)
	{
		const FKey* Key = ActionKeys.Find(Action);
		return Key && WasInputKeyJustPressed(*Key);
	};
	const auto Released = [this](const TCHAR* Action)
	{
		const FKey* Key = ActionKeys.Find(Action);
		return Key && WasInputKeyJustReleased(*Key);
	};
	const auto Down = [this](const TCHAR* Action)
	{
		const FKey* Key = ActionKeys.Find(Action);
		return Key && IsInputKeyDown(*Key);
	};
	if (Pressed(TEXT("board")))
	{
		ServerBoard();
	}
	const AFMShip* Ship = AFMShip::Find(GetWorld());
	if (!Ship)
	{
		return;
	}
	const FFMShipState& S = Ship->GetState();
	if (Pressed(TEXT("wheel_right")))
	{
		DriveByKey(TEXT("wheel"), 1.0f);
	}
	if (Pressed(TEXT("wheel_left")))
	{
		DriveByKey(TEXT("wheel"), -1.0f);
	}
	if ((Released(TEXT("wheel_right")) || Released(TEXT("wheel_left"))) && !Down(TEXT("wheel_right")) && !Down(TEXT("wheel_left")))
	{
		DriveByKey(TEXT("wheel"), S.Rudder);
	}
	if (Pressed(TEXT("sail_down")))
	{
		DriveByKey(TEXT("sail_length"), 1.0f);
	}
	if (Pressed(TEXT("sail_up")))
	{
		DriveByKey(TEXT("sail_length"), 0.0f);
	}
	if (Released(TEXT("sail_up")) || Released(TEXT("sail_down")))
	{
		DriveByKey(TEXT("sail_length"), S.SailLength);
	}
	if (Pressed(TEXT("angle_right")))
	{
		DriveByKey(TEXT("sail_angle"), 90.0f);
	}
	if (Pressed(TEXT("angle_left")))
	{
		DriveByKey(TEXT("sail_angle"), -90.0f);
	}
	if (Released(TEXT("angle_right")) || Released(TEXT("angle_left")))
	{
		DriveByKey(TEXT("sail_angle"), S.SailAngle);
	}
	if (Pressed(TEXT("anchor")))
	{
		DriveByKey(TEXT("anchor"), Ship->GetInputs().bAnchorDown ? 0.0f : 1.0f);
	}
	if (Pressed(TEXT("ladder")))
	{
		DriveByKey(TEXT("ladder"), 1.0f);
	}
}

void AFMPlayerController::DriveByKey(FName Station, float Value)
{
	float Distance = 0.0f;
	if (!WithinStation(Station, Distance))
	{
		Notify(Distance >= 0.0f
			? FString::Printf(TEXT("%s: out of radius, %.1f m away"), *Station.ToString(), Distance / 100.0f)
			: FString::Printf(TEXT("%s: off the ship"), *Station.ToString()));
	}
	DriveShip(Station, Value);
}

bool AFMPlayerController::WithinStation(FName Station, float& OutDistance) const
{
	OutDistance = -1.0f;
	const AFMShip* Ship = AFMShip::Find(GetWorld());
	const APawn* Controlled = GetPawn();
	const FFMStation* Place = GetDefault<UFMShipSettings>()->Stations.FindByPredicate([&](const FFMStation& Candidate) { return Candidate.Name == Station; });
	if (!Ship || !Controlled || !Place)
	{
		return false;
	}
	OutDistance = Ship->StationDistance(*Place, *Controlled);
	return OutDistance <= Place->Radius;
}

void AFMPlayerController::Notify(const FString& Text)
{
	NoticeText = Text;
	NoticeUntil = GetWorld()->GetTimeSeconds() + NoticeSeconds;
}

FString AFMPlayerController::Notice() const
{
	return GetWorld() && GetWorld()->GetTimeSeconds() < NoticeUntil ? NoticeText : FString();
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
		Ship->Apply(Input, Value, GetPawn());
	}
}

void AFMPlayerController::ServerBoard_Implementation()
{
	AFMShip* Ship = AFMShip::Find(GetWorld());
	AFMPlayerPawn* Controlled = Cast<AFMPlayerPawn>(GetPawn());
	if (Ship && Controlled)
	{
		Ship->Board(*Controlled);
	}
}

void AFMPlayerController::ServerRelayTrace_Implementation(const TArray<FString>& Lines)
{
	if (UFMTraceSubsystem* TraceSubsystem = UFMTraceSubsystem::Get(this))
	{
		TraceSubsystem->Ingest(Lines);
	}
}
