#include "Deck/FMPlayerPawn.h"

#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/FMPlayerController.h"
#include "Deck/FMSwimMode.h"
#include "DefaultMovementSet/CharacterMoverComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/PlayerState.h"
#include "MoverDataModelTypes.h"
#include "Net/FMTrace.h"
#include "Ocean/FMOceanSubsystem.h"
#include "UObject/ConstructorHelpers.h"

bool FFMTeleportEffect::ApplyMovementEffect(FApplyMovementEffectParams& ApplyEffectParams, FMoverSyncState& OutputState)
{
	const bool bApplied = FTeleportEffect::ApplyMovementEffect(ApplyEffectParams, OutputState);
	if (bApplied)
	{
		OutputState.MovementMode = DefaultModeNames::Falling;
	}
	return bApplied;
}

FInstantMovementEffect* FFMTeleportEffect::Clone() const
{
	return new FFMTeleportEffect(*this);
}

UScriptStruct* FFMTeleportEffect::GetScriptStruct() const
{
	return FFMTeleportEffect::StaticStruct();
}

AFMPlayerPawn::AFMPlayerPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicatingMovement(false);
	bUseControllerRotationYaw = false;

	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->InitCapsuleSize(34.0f, 88.0f);
	Capsule->SetCollisionProfileName(TEXT("Pawn"));
	Capsule->SetCanEverAffectNavigation(false);
	RootComponent = Capsule;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Capsule);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderMesh.Succeeded())
	{
		Mesh->SetStaticMesh(CylinderMesh.Object);
	}
	Mesh->SetRelativeScale3D(FVector(0.68f, 0.68f, 1.76f));
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetCanEverAffectNavigation(false);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(Capsule);
	Camera->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f));
	Camera->bUsePawnControlRotation = true;

	Mover = CreateDefaultSubobject<UCharacterMoverComponent>(TEXT("Mover"));
	Mover->MovementModes.Add(DefaultModeNames::Swimming, CreateDefaultSubobject<UFMSwimMode>(TEXT("SwimMode")));
	Mover->Transitions.Add(CreateDefaultSubobject<UFMSwimTransition>(TEXT("SwimTransition")));
}

void AFMPlayerPawn::BeginPlay()
{
	Super::BeginPlay();
	Mover->OnPostFinalize.AddDynamic(this, &AFMPlayerPawn::HandlePostFinalize);
	Mover->OnPostSimulationRollback.AddDynamic(this, &AFMPlayerPawn::HandleRollback);
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (PC->PlayerCameraManager)
		{
			PC->PlayerCameraManager->ViewPitchMax = 89.0f;
			PC->PlayerCameraManager->ViewPitchMin = -89.0f;
		}
	}
}

void AFMPlayerPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC && PC->IsLocalController())
	{
		float DeltaX = 0.0f, DeltaY = 0.0f;
		PC->GetInputMouseDelta(DeltaX, DeltaY);
		PC->AddYawInput(DeltaX * LookScale);
		PC->AddPitchInput(-DeltaY * LookScale);
	}
}

void AFMPlayerPawn::SetHarnessRole(FName InRole)
{
	HarnessRole = InRole;
}

FString AFMPlayerPawn::RoleName() const
{
	if (!HarnessRole.IsNone())
	{
		return HarnessRole.ToString();
	}
	const APlayerState* State = GetPlayerState();
	return FString::Printf(TEXT("pid%d"), State ? State->GetPlayerId() : -1);
}

void AFMPlayerPawn::HarnessTeleport(FVector Location, float Yaw)
{
	TSharedPtr<FFMTeleportEffect> Effect = MakeShared<FFMTeleportEffect>();
	Effect->TargetLocation = Location;
	Effect->bUseActorRotation = false;
	Effect->TargetRotation = FRotator(0.0f, Yaw, 0.0f);
	Mover->QueueInstantMovementEffect(Effect);
}

void AFMPlayerPawn::DriveShip(FName Input, float Value)
{
	if (AFMPlayerController* PC = Cast<AFMPlayerController>(GetController()))
	{
		PC->DriveShip(Input, Value);
	}
}

FVector AFMPlayerPawn::GetSimLocation() const
{
	const FMoverDefaultSyncState* State = Mover->GetSyncState().SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();
	return State ? State->GetLocation_WorldSpace() : GetActorLocation();
}

int32 AFMPlayerPawn::GetSimFrame() const
{
	return Mover->GetLastTimeStep().ServerFrame;
}

void AFMPlayerPawn::ProduceInput_Implementation(int32 SimTimeMs, FMoverInputCmdContext& InputCmdResult)
{
	FCharacterDefaultInputs& Inputs = InputCmdResult.InputCollection.FindOrAddMutableDataByType<FCharacterDefaultInputs>();
	const AFMPlayerController* PC = Cast<AFMPlayerController>(GetController());
	if (!PC || !PC->IsLocalController())
	{
		Inputs = FCharacterDefaultInputs();
		return;
	}

	for (const TPair<FName, FKey>& Binding : PC->ActionKeys)
	{
		const bool bDown = PC->IsInputKeyDown(Binding.Value);
		bool& bWasDown = KeyWasDown.FindOrAdd(Binding.Key);
		if (bDown != bWasDown)
		{
			bWasDown = bDown;
			FM_TRACE(this, TEXT("INPUT role=%s action=%s edge=%s"),
				*RoleName(), *Binding.Key.ToString(), bDown ? TEXT("pressed") : TEXT("released"));
		}
	}

	const auto Down = [&](const TCHAR* Action)
	{
		const FKey* Key = PC->ActionKeys.Find(FName(Action));
		return Key && PC->IsInputKeyDown(*Key);
	};
	FVector Intent = FVector::ZeroVector;
	Intent.X += Down(TEXT("move_forward")) ? 1.0f : 0.0f;
	Intent.X -= Down(TEXT("move_back")) ? 1.0f : 0.0f;
	Intent.Y += Down(TEXT("move_right")) ? 1.0f : 0.0f;
	Intent.Y -= Down(TEXT("move_left")) ? 1.0f : 0.0f;

	const FRotator Control = PC->GetControlRotation();
	const FRotator YawOnly(0.0f, Control.Yaw, 0.0f);
	Inputs.ControlRotation = Control;
	Inputs.SetMoveInput(EMoveInputType::DirectionalIntent, YawOnly.RotateVector(Intent.GetClampedToMaxSize(1.0f)));
	Inputs.OrientationIntent = YawOnly.Vector();
	Inputs.SuggestedMovementMode = NAME_None;
	Inputs.bUsingMovementBase = false;

	const bool bJump = Down(TEXT("jump"));
	Inputs.bIsJumpJustPressed = bJump && !bJumpWasDown;
	Inputs.bIsJumpPressed = bJump;
	bJumpWasDown = bJump;
}

void AFMPlayerPawn::HandlePostFinalize(const FMoverSyncState& SyncState, const FMoverAuxStateContext& AuxState)
{
	const APlayerState* Player = GetPlayerState();
	if (PendingRollbackTo >= 0 && Player)
	{
		FM_TRACE(this, TEXT("ROLLBACK pid=%d n=%d to=%d from=%d"), Player->GetPlayerId(), Rollbacks, PendingRollbackTo, PendingRollbackFrom);
		PendingRollbackTo = PendingRollbackFrom = -1;
	}
	const int32 SimFrame = Mover->GetLastTimeStep().ServerFrame;
	if (SimFrame < 0 || PoseEveryFrames <= 0 || SimFrame % PoseEveryFrames != 0)
	{
		return;
	}
	const FMoverDefaultSyncState* State = SyncState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();
	if (!State || !Player)
	{
		return;
	}
	const FVector Location = State->GetLocation_WorldSpace();
	const bool bBased = State->GetMovementBase() != nullptr;
	const FVector BaseSpace = bBased ? State->GetLocation_BaseSpace() : FVector::ZeroVector;
	const UFMOceanSubsystem* Ocean = UFMOceanSubsystem::Get(this);
	const float Water = GetDefault<UFMOceanSettings>()->PlaneZ + (Ocean ? Ocean->HeightAt(FVector2f(Location.X, Location.Y), SimFrame) : 0.0f);
	FM_TRACE(this, TEXT("POSE pid=%d sf=%d x=%.2f y=%.2f z=%.2f yaw=%.1f mode=%s base=%d bx=%.2f by=%.2f bz=%.2f wz=%.2f"),
		Player->GetPlayerId(), SimFrame, Location.X, Location.Y, Location.Z,
		State->GetOrientation_WorldSpace().Yaw, *SyncState.MovementMode.ToString(),
		bBased ? 1 : 0, BaseSpace.X, BaseSpace.Y, BaseSpace.Z, Water);
}

void AFMPlayerPawn::HandleRollback(const FMoverTimeStep& CurrentTimeStep, const FMoverTimeStep& ExpungedTimeStep)
{
	++Rollbacks;
	PendingRollbackTo = CurrentTimeStep.ServerFrame;
	PendingRollbackFrom = FMath::Max(PendingRollbackFrom, ExpungedTimeStep.ServerFrame);
}
