#include "Deck/FMPlayerPawn.h"

#include "Animation/AnimSequence.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Combat/FMCombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/FMPlayerController.h"
#include "Core/FMPlayerState.h"
#include "Deck/FMSwimMode.h"
#include "DefaultMovementSet/CharacterMoverComponent.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/PlayerState.h"
#include "MoverDataModelTypes.h"
#include "Net/FMTrace.h"
#include "Ocean/FMOceanSubsystem.h"
#include "Ship/FMShip.h"

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

	Visual = CreateDefaultSubobject<USceneComponent>(TEXT("Visual"));
	Visual->SetupAttachment(Capsule);

	ArmsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ArmsMesh"));
	ArmsMesh->SetupAttachment(Visual);
	ArmsMesh->SetOnlyOwnerSee(true);
	BodyMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(Visual);
	BodyMesh->SetOwnerNoSee(true);
	for (USkeletalMeshComponent* Mesh : { ArmsMesh.Get(), BodyMesh.Get() })
	{
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Mesh->SetCanEverAffectNavigation(false);
		Mesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	}

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(Visual);
	Camera->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f));
	Camera->bUsePawnControlRotation = true;

	Mover = CreateDefaultSubobject<UCharacterMoverComponent>(TEXT("Mover"));
	Mover->MovementModes.Add(DefaultModeNames::Swimming, CreateDefaultSubobject<UFMSwimMode>(TEXT("SwimMode")));
	Mover->Transitions.Add(CreateDefaultSubobject<UFMSwimTransition>(TEXT("SwimTransition")));
	Mover->PersistentSyncStateDataTypes.Add(FMoverDataPersistence(FFMCombatState::StaticStruct(), true));

	Combat = CreateDefaultSubobject<UFMCombatComponent>(TEXT("Combat"));
}

void AFMPlayerPawn::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	Mover->SetPrimaryVisualComponent(Visual);
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
	const UFMCombatSettings* K = GetDefault<UFMCombatSettings>();
	Camera->SetRelativeLocation(K->EyeOffset);
	const TPair<USkeletalMeshComponent*, USkeletalMesh*> Rigs[] = {
		{ ArmsMesh.Get(), K->FirstPersonMesh.LoadSynchronous() },
		{ BodyMesh.Get(), K->ThirdPersonMesh.LoadSynchronous() } };
	for (const TPair<USkeletalMeshComponent*, USkeletalMesh*>& Rig : Rigs)
	{
		Rig.Key->SetRelativeLocationAndRotation(K->MeshOffset, FRotator(0.0f, K->MeshYaw, 0.0f));
		if (Rig.Value)
		{
			Rig.Key->SetSkeletalMeshAsset(Rig.Value);
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
	if (!IsNetMode(NM_DedicatedServer))
	{
		Pose(ArmsMesh, true);
		Pose(BodyMesh, false);
		Combat->DrawPending();
	}
}

bool AFMPlayerPawn::ShipSpaceLocation(FVector& Out) const
{
	const FMoverDefaultSyncState* State = Mover->GetSyncState().SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();
	if (!State || !State->GetMovementBase())
	{
		return false;
	}
	Out = State->GetLocation_BaseSpace();
	return true;
}

FString AFMPlayerPawn::MovementModeName() const
{
	return Mover->GetSyncState().MovementMode.ToString();
}

void AFMPlayerPawn::Pose(USkeletalMeshComponent* Target, bool bFirstPerson)
{
	const UAnimSequence* Clip = nullptr;
	float Time = 0.0f;
	if (!Combat->Presented(bFirstPerson, Clip, Time))
	{
		const UFMAttackData* Idle = Combat->AttackData(1);
		Clip = Idle ? (bFirstPerson ? Idle->FirstPerson.Get() : Idle->ThirdPerson.Get()) : nullptr;
		Time = 0.0f;
	}
	if (!Clip)
	{
		return;
	}
	const UAnimSequence*& Current = Posed.FindOrAdd(Target);
	if (Current != Clip)
	{
		Current = Clip;
		Target->SetAnimation(const_cast<UAnimSequence*>(Clip));
	}
	Target->SetPosition(Time, false);
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
	FFMCombatInputs& CombatInputs = InputCmdResult.InputCollection.FindOrAddMutableDataByType<FFMCombatInputs>();
	AFMPlayerController* PC = Cast<AFMPlayerController>(GetController());
	if (!PC || !PC->IsLocalController())
	{
		Inputs = FCharacterDefaultInputs();
		CombatInputs = FFMCombatInputs();
		return;
	}

	TSet<FName> Latched;
	PC->TakeLatched(Latched);
	TSet<FName> Pressed;
	for (const TPair<FName, FKey>& Binding : PC->ActionKeys)
	{
		const bool bLatched = Latched.Contains(Binding.Key);
		if (AFMPlayerController::IsMomentary(Binding.Value))
		{
			if (bLatched)
			{
				Pressed.Add(Binding.Key);
				FM_TRACE(this, TEXT("INPUT role=%s action=%s edge=pressed"), *RoleName(), *Binding.Key.ToString());
			}
			continue;
		}
		const bool bDown = bLatched || PC->IsActionDown(Binding.Key);
		bool& bWasDown = KeyWasDown.FindOrAdd(Binding.Key);
		if (bDown != bWasDown)
		{
			bWasDown = bDown;
			if (bDown)
			{
				Pressed.Add(Binding.Key);
			}
			FM_TRACE(this, TEXT("INPUT role=%s action=%s edge=%s"),
				*RoleName(), *Binding.Key.ToString(), bDown ? TEXT("pressed") : TEXT("released"));
		}
	}

	const auto Down = [&](const TCHAR* Action)
	{
		const FName Name(Action);
		return Latched.Contains(Name) || PC->IsActionDown(Name);
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

	if (bHasLastControlYaw)
	{
		const float DeltaYaw = FMath::FindDeltaAngleDegrees(LastControlYaw, Control.Yaw);
		if (FMath::Abs(DeltaYaw) > 0.05f)
		{
			Side = DeltaYaw > 0.0f ? EFMAttackSide::Right : EFMAttackSide::Left;
		}
	}
	LastControlYaw = Control.Yaw;
	bHasLastControlYaw = true;

	CombatInputs = FFMCombatInputs();
	if (Pressed.Contains(TEXT("attack_thrust")))
	{
		CombatInputs.Attack = EFMAttackType::Thrust;
	}
	else if (Pressed.Contains(TEXT("attack_overhead")))
	{
		CombatInputs.Attack = EFMAttackType::Overhead;
	}
	else if (Pressed.Contains(TEXT("attack_horizontal")))
	{
		CombatInputs.Attack = EFMAttackType::Horizontal;
	}
	CombatInputs.Side = Side;
	CombatInputs.bParry = Pressed.Contains(TEXT("parry"));
	CombatInputs.bFeint = Pressed.Contains(TEXT("feint"));
	float RenderedFraction = 0.0f;
	CombatInputs.RenderedFrame = UFMCombatComponent::RenderedFrame(GetWorld(), RenderedFraction);
	CombatInputs.RenderedFraction = static_cast<uint8>(FMath::RoundToInt(RenderedFraction * 255.0f));
	const AFMPlayerState* Player = GetPlayerState<AFMPlayerState>();
	CombatInputs.AdvanceFrames = static_cast<uint8>(FMath::Clamp(Player ? Player->AdvanceFrames : 0, 0, 255));
}

void AFMPlayerPawn::HandlePostFinalize(const FMoverSyncState& SyncState, const FMoverAuxStateContext& AuxState)
{
	const FMoverDefaultSyncState* State = SyncState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();
	const UPrimitiveComponent* Base = State ? State->GetMovementBase() : nullptr;
	if (Base && GetLocalRole() == ROLE_SimulatedProxy)
	{
		PlaceOnBase(*State, *Base);
	}
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
	if (!State || !Player)
	{
		return;
	}
	const FVector Location = State->GetLocation_WorldSpace();
	const FVector BaseSpace = Base ? State->GetLocation_BaseSpace() : FVector::ZeroVector;
	const FVector Rendered = Base ? PresentedBase(*Base).InverseTransformPositionNoScale(Visual->GetComponentLocation()) : FVector::ZeroVector;
	const UFMOceanSubsystem* Ocean = UFMOceanSubsystem::Get(this);
	const float Water = GetDefault<UFMOceanSettings>()->PlaneZ + (Ocean ? Ocean->HeightAt(FVector2f(Location.X, Location.Y), SimFrame) : 0.0f);
	FM_TRACE(this, TEXT("POSE pid=%d sf=%d x=%.2f y=%.2f z=%.2f yaw=%.1f mode=%s base=%d bx=%.2f by=%.2f bz=%.2f rx=%.2f ry=%.2f rz=%.2f wz=%.2f"),
		Player->GetPlayerId(), SimFrame, Location.X, Location.Y, Location.Z,
		State->GetOrientation_WorldSpace().Yaw, *SyncState.MovementMode.ToString(),
		Base ? 1 : 0, BaseSpace.X, BaseSpace.Y, BaseSpace.Z, Rendered.X, Rendered.Y, Rendered.Z, Water);
}

FTransform AFMPlayerPawn::PresentedBase(const UPrimitiveComponent& Base)
{
	const AFMShip* Ship = Cast<AFMShip>(Base.GetOwner());
	return Ship ? Ship->PresentedTransform() : Base.GetComponentTransform();
}

void AFMPlayerPawn::PlaceOnBase(const FMoverDefaultSyncState& State, const UPrimitiveComponent& Base)
{
	const FTransform BaseNow = PresentedBase(Base);
	const FVector Location = BaseNow.TransformPositionNoScale(State.GetLocation_BaseSpace());
	const float YawTurned = BaseNow.Rotator().Yaw - State.GetCapturedMovementBaseQuat().Rotator().Yaw;
	const FQuat Orientation = FRotator(0.0f, YawTurned, 0.0f).Quaternion() * State.GetOrientation_WorldSpace().Quaternion();
	SetActorLocationAndRotation(Location, Orientation, false, nullptr, ETeleportType::TeleportPhysics);
}

void AFMPlayerPawn::HandleRollback(const FMoverTimeStep& CurrentTimeStep, const FMoverTimeStep& ExpungedTimeStep)
{
	++Rollbacks;
	PendingRollbackTo = CurrentTimeStep.ServerFrame;
	PendingRollbackFrom = FMath::Max(PendingRollbackFrom, ExpungedTimeStep.ServerFrame);
}
