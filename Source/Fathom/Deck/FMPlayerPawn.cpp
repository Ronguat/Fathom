#include "Deck/FMPlayerPawn.h"

#include "Animation/AnimSequence.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Combat/FMCombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/FMGameState.h"
#include "Core/FMPlayerController.h"
#include "Core/FMPlayerState.h"
#include "Deck/FMDeckModes.h"
#include "Deck/FMSwimMode.h"
#include "DefaultMovementSet/CharacterMoverComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/PlayerState.h"
#include "MoverDataModelTypes.h"
#include "Net/FMTrace.h"
#include "NetworkPredictionWorldManager.h"
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
	: Super(ObjectInitializer
		.SetNestedDefaultSubobjectClass<UFMDeckWalkingMode>(TEXT("Mover.DefaultWalkingMode"))
		.SetNestedDefaultSubobjectClass<UFMDeckFallingMode>(TEXT("Mover.DefaultFallingMode")))
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
	ArmsWeapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ArmsWeapon"));
	ArmsWeapon->SetupAttachment(ArmsMesh);
	ArmsWeapon->SetOnlyOwnerSee(true);
	BodyWeapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BodyWeapon"));
	BodyWeapon->SetupAttachment(BodyMesh);
	BodyWeapon->SetOwnerNoSee(true);
	ArmsWeapon->SetForceRefPose(true);
	BodyWeapon->SetForceRefPose(true);
	for (USkeletalMeshComponent* Mesh : { ArmsMesh.Get(), BodyMesh.Get(), ArmsWeapon.Get(), BodyWeapon.Get() })
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
	Mover->SmoothingMode = EMoverSmoothingMode::None;
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
	Mover->OnPreSimulationTick.AddDynamic(this, &AFMPlayerPawn::HandlePreSimulationTick);
	Mover->OnPostFinalize.AddDynamic(this, &AFMPlayerPawn::HandlePostFinalize);
	Mover->OnPostSimulationRollback.AddDynamic(this, &AFMPlayerPawn::HandleRollback);
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (PC->PlayerCameraManager)
		{
			PC->PlayerCameraManager->ViewPitchMax = 89.0f;
			PC->PlayerCameraManager->ViewPitchMin = -89.0f;
			PC->PlayerCameraManager->ViewRollMax = 0.0f;
			PC->PlayerCameraManager->ViewRollMin = 0.0f;
		}
	}
	const UFMCombatSettings* K = GetDefault<UFMCombatSettings>();
	Camera->SetRelativeLocation(K->EyeOffset);
	const TPair<USkeletalMeshComponent*, USkeletalMesh*> Rigs[] = {
		{ ArmsMesh.Get(), K->FirstPersonMesh.LoadSynchronous() },
		{ BodyMesh.Get(), K->ThirdPersonMesh.LoadSynchronous() } };
	for (const TPair<USkeletalMeshComponent*, USkeletalMesh*>& Rig : Rigs)
	{
		Rig.Key->SetRelativeLocationAndRotation(K->MeshOffset + (Rig.Key == ArmsMesh ? K->ArmsOffset : FVector::ZeroVector), FRotator(0.0f, K->MeshYaw, 0.0f));
		if (Rig.Value)
		{
			Rig.Key->SetSkeletalMeshAsset(Rig.Value);
		}
	}
	USkeletalMesh* WeaponAsset = K->WeaponMesh.LoadSynchronous();
	const TPair<USkeletalMeshComponent*, USkeletalMeshComponent*> Hands[] = {
		{ ArmsWeapon.Get(), ArmsMesh.Get() },
		{ BodyWeapon.Get(), BodyMesh.Get() } };
	for (const TPair<USkeletalMeshComponent*, USkeletalMeshComponent*>& Hand : Hands)
	{
		Hand.Key->AttachToComponent(Hand.Value, FAttachmentTransformRules::KeepRelativeTransform, K->WeaponSocket);
		Hand.Key->SetRelativeLocationAndRotation(K->WeaponOffset, K->WeaponRotation);
		if (WeaponAsset)
		{
			Hand.Key->SetSkeletalMeshAsset(WeaponAsset);
		}
	}
	if (!IsNetMode(NM_DedicatedServer))
	{
		BodyMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
		ArmsWeapon->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
		BodyWeapon->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	}
	FirstPersonIdle = K->FirstPersonIdle.LoadSynchronous();
	ThirdPersonIdle = K->ThirdPersonIdle.LoadSynchronous();
	FirstPersonWalk = K->FirstPersonWalk.LoadSynchronous();
	ThirdPersonWalk = K->ThirdPersonWalk.LoadSynchronous();
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
		LoopSeconds += DeltaSeconds;
		if (GetLocalRole() != ROLE_SimulatedProxy)
		{
			SmoothVisual();
		}
		Pose(ArmsMesh, true);
		Pose(BodyMesh, false);
		TraceBlade();
		Combat->DrawPending();
	}
}

void AFMPlayerPawn::SmoothVisual()
{
	if (SmoothFrame < 0)
	{
		return;
	}
	const UNetworkPredictionWorldManager* Prediction = GetWorld()->GetSubsystem<UNetworkPredictionWorldManager>();
	float Fraction = 1.0f;
	if (Prediction)
	{
		const FFixedTickState& Tick = Prediction->GetFixedTickState();
		Fraction = FMath::Clamp(Tick.UnspentTimeMS / FMath::Max(1.0f, Tick.FixedStepRealTimeMS), 0.0f, 1.0f);
	}
	const FVector Local = FMath::Lerp(SmoothFrom, SmoothTo, Fraction);
	const float Yaw = SmoothYawFrom + FMath::FindDeltaAngleDegrees(SmoothYawFrom, SmoothYawTo) * Fraction;
	const UPrimitiveComponent* Base = SmoothBase.Get();
	const FVector Location = Base ? PresentedBase(*Base).TransformPositionNoScale(Local) : Local;
	const float WorldYaw = Base ? Yaw + PresentedBase(*Base).Rotator().Yaw : Yaw;
	Visual->SetWorldLocationAndRotation(Location, FRotator(0.0f, WorldYaw, 0.0f));
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
	float AttackFrame = -1.0f;
	if (!bFirstPerson)
	{
		bPosedRelease = false;
		PosedAttack = 0;
	}
	if (Combat->Presented(bFirstPerson, Clip, Time, AttackFrame))
	{
		if (!bFirstPerson && AttackFrame >= 0.0f)
		{
			const FFMCombatState* S = Combat->State();
			const UFMAttackData* Data = S ? Combat->AttackData(S->Attack) : nullptr;
			PosedAttack = S ? S->Attack : 0;
			PosedAttackFrame = AttackFrame;
			bPosedRelease = Data && AttackFrame >= Data->WindupFrames && AttackFrame < Data->WindupFrames + Data->ReleaseFrames;
		}
	}
	else
	{
		const FMoverDefaultSyncState* State = Mover->GetSyncState().SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();
		const FVector Velocity = State ? (State->GetMovementBase() ? State->GetVelocity_BaseSpace() : State->GetVelocity_WorldSpace()) : FVector::ZeroVector;
		const bool bWalking = Velocity.Size2D() > GetDefault<UFMCombatSettings>()->WalkSpeedMin;
		Clip = bWalking ? (bFirstPerson ? FirstPersonWalk : ThirdPersonWalk) : (bFirstPerson ? FirstPersonIdle : ThirdPersonIdle);
		if (Clip)
		{
			Time = FMath::Fmod(LoopSeconds, FMath::Max(Clip->GetPlayLength(), 0.001f));
		}
		else
		{
			const UFMAttackData* Idle = Combat->AttackData(1);
			Clip = Idle ? (bFirstPerson ? Idle->FirstPerson.Get() : Idle->ThirdPerson.Get()) : nullptr;
		}
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
	if (Target == BodyMesh || IsLocallyControlled())
	{
		Target->TickAnimation(0.0f, false);
		Target->RefreshBoneTransforms();
		Target->UpdateChildTransforms();
	}
}

void AFMPlayerPawn::TraceBlade()
{
	if (!bPosedRelease)
	{
		return;
	}
	const UFMCombatSettings* K = GetDefault<UFMCombatSettings>();
	const UFMAttackData* Data = Combat->AttackData(PosedAttack);
	TArray<FVector> Baked;
	if (!Data || !BodyWeapon->GetSkeletalMeshAsset() || !Data->TracersBetween(PosedAttackFrame, Baked) || Baked.Num() < 2)
	{
		return;
	}
	const FTransform Frame = Combat->PresentedPawnFrame();
	const auto Drawn = [&](const USkeletalMeshComponent* Weapon, TArray<FVector>& Out)
	{
		const FVector Base = Frame.InverseTransformPosition(Weapon->GetSocketLocation(K->BladeBaseSocket));
		const FVector Tip = Frame.InverseTransformPosition(Weapon->GetSocketLocation(K->BladeTipSocket));
		Out.SetNumUninitialized(Baked.Num());
		for (int32 T = 0; T < Baked.Num(); ++T)
		{
			Out[T] = FMath::Lerp(Base, Tip, static_cast<float>(T) / (Baked.Num() - 1));
		}
	};
	TArray<FVector> Body;
	Drawn(BodyWeapon, Body);
	const FTransform Hand = BodyMesh->GetSocketTransform(K->WeaponSocket);
	const FTransform Ridden = FTransform(K->WeaponRotation, K->WeaponOffset) * Hand;
	float Attach = 0.0f;
	for (const FName& Socket : { K->BladeBaseSocket, K->BladeTipSocket })
	{
		const FVector Local = BodyWeapon->GetSocketTransform(Socket, RTS_Component).GetLocation();
		Attach = FMath::Max(Attach, static_cast<float>((Ridden.TransformPosition(Local) - BodyWeapon->GetSocketLocation(Socket)).Size()));
	}
	float ErrMax = 0.0f;
	for (int32 T = 0; T < Baked.Num(); ++T)
	{
		ErrMax = FMath::Max(ErrMax, static_cast<float>((Body[T] - Baked[T]).Size()));
	}
	float FpBase = -1.0f, FpTip = -1.0f;
	if (IsLocallyControlled() && ArmsWeapon->GetSkeletalMeshAsset())
	{
		TArray<FVector> Arms;
		Drawn(ArmsWeapon, Arms);
		FpBase = static_cast<float>((Arms[0] - Baked[0]).Size());
		FpTip = static_cast<float>((Arms.Last() - Baked.Last()).Size());
		if (UFMCombatComponent::DrawEnabled())
		{
			DrawDebugLine(GetWorld(), Frame.TransformPosition(Arms[0]), Frame.TransformPosition(Arms.Last()), FColor::Orange, false, -1.0f, 0, 1.5f);
		}
	}
	if (UFMCombatComponent::DrawEnabled())
	{
		for (const FVector& Point : Body)
		{
			DrawDebugPoint(GetWorld(), Frame.TransformPosition(Point), 8.0f, FColor::Yellow, false, -1.0f, 0);
		}
	}
	const APlayerState* Player = GetPlayerState();
	FM_TRACE(this, TEXT("BLADE pid=%d k=%.2f attack=%s n=%d err_max=%.1f err_base=%.1f err_tip=%.1f att=%.1f pos=%.2f fp_base=%.1f fp_tip=%.1f"),
		Player ? Player->GetPlayerId() : -1, PosedAttackFrame, *Data->AttackName(), Baked.Num(), ErrMax,
		static_cast<float>((Body[0] - Baked[0]).Size()), static_cast<float>((Body.Last() - Baked.Last()).Size()), Attach, BodyMesh->GetPosition() * 60.0f, FpBase, FpTip);
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
	PendingStations.Emplace(Input, Value);
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
	FFMStationInputs& StationInputs = InputCmdResult.InputCollection.FindOrAddMutableDataByType<FFMStationInputs>();
	StationInputs = FFMStationInputs();
	AFMPlayerController* PC = Cast<AFMPlayerController>(GetController());
	if (!PC || !PC->IsLocalController())
	{
		Inputs = FCharacterDefaultInputs();
		CombatInputs = FFMCombatInputs();
		return;
	}
	if (PendingStations.Num() > 0)
	{
		const AFMGameState* Session = GetWorld()->GetGameState<AFMGameState>();
		StationInputs.Station = AFMShip::StationIndex(PendingStations[0].Key);
		StationInputs.Value = PendingStations[0].Value;
		StationInputs.Delay = static_cast<uint8>(FMath::Clamp(Session ? Session->StationDelayFrames : 0, 0, 255));
		PendingStations.RemoveAt(0);
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
	Inputs.SuggestedMovementMode = NAME_None;
	if (Pressed.Contains(TEXT("fly")))
	{
		bFlying = !bFlying;
		Inputs.SuggestedMovementMode = bFlying ? DefaultModeNames::Flying : DefaultModeNames::Falling;
	}
	const FRotator MoveFrame = bFlying ? FRotator(Control.Pitch, Control.Yaw, 0.0f) : YawOnly;
	Inputs.SetMoveInput(EMoveInputType::DirectionalIntent, MoveFrame.RotateVector(Intent.GetClampedToMaxSize(1.0f)));
	Inputs.OrientationIntent = YawOnly.Vector();
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

void AFMPlayerPawn::HandlePreSimulationTick(const FMoverTimeStep& TimeStep, const FMoverInputCmdContext& InputCmd)
{
	if (!Ship.IsValid())
	{
		Ship = AFMShip::Find(GetWorld());
	}
	if (Ship.IsValid())
	{
		Ship->AdvanceTo(TimeStep.ServerFrame);
		const FFMStationInputs* Station = InputCmd.InputCollection.FindDataByType<FFMStationInputs>();
		if (Station && Station->Station != 0)
		{
			Ship->Apply(AFMShip::StationName(Station->Station), Station->Value, this, Station->Delay);
		}
	}
}

void AFMPlayerPawn::HandlePostFinalize(const FMoverSyncState& SyncState, const FMoverAuxStateContext& AuxState)
{
	const FMoverDefaultSyncState* State = SyncState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();
	const UPrimitiveComponent* Base = State ? State->GetMovementBase() : nullptr;
	if (State && GetLocalRole() == ROLE_SimulatedProxy)
	{
		if (Base)
		{
			PlaceOnBase(*State, *Base);
		}
		DrawProxy(GetActorLocation(), Base != nullptr);
	}
	const int32 Finalized = Mover->GetLastTimeStep().ServerFrame;
	if (APlayerController* PC = Cast<APlayerController>(GetController()); PC && PC->IsLocalController() && Finalized != SmoothFrame)
	{
		const float BaseYaw = Base ? Base->GetComponentRotation().Yaw : 0.0f;
		if (Base && bHasBaseYaw && SmoothBase.Get() == Base)
		{
			const float Carry = FMath::FindDeltaAngleDegrees(BaseYawSeen, BaseYaw);
			PC->SetControlRotation(PC->GetControlRotation() + FRotator(0.0f, Carry, 0.0f));
			LastControlYaw = FRotator::NormalizeAxis(LastControlYaw + Carry);
		}
		BaseYawSeen = BaseYaw;
		bHasBaseYaw = Base != nullptr;
	}
	if (State && GetLocalRole() != ROLE_SimulatedProxy && Finalized != SmoothFrame)
	{
		const FVector Local = Base ? State->GetLocation_BaseSpace() : State->GetLocation_WorldSpace();
		const float Yaw = Base ? State->GetOrientation_BaseSpace().Yaw : State->GetOrientation_WorldSpace().Yaw;
		const bool bContinuous = SmoothFrame >= 0 && Finalized == SmoothFrame + 1 && SmoothBase.Get() == Base;
		SmoothFrom = bContinuous ? SmoothTo : Local;
		SmoothYawFrom = bContinuous ? SmoothYawTo : Yaw;
		SmoothTo = Local;
		SmoothYawTo = Yaw;
		SmoothBase = Base;
		SmoothFrame = Finalized;
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

void AFMPlayerPawn::DrawProxy(const FVector& Placed, bool bBased)
{
	if (bProxyDrawn && bBased != bProxyWasBased)
	{
		const FVector Gap = ProxyDrawn - Placed;
		ProxyGap = Gap.Size() <= ProxySnapMax ? Gap : FVector::ZeroVector;
	}
	else if (!ProxyGap.IsNearlyZero())
	{
		const float Step = ProxySnapDecay * GetWorld()->GetDeltaSeconds();
		ProxyGap = ProxyGap.Size() <= Step ? FVector::ZeroVector : ProxyGap - ProxyGap.GetSafeNormal() * Step;
	}
	ProxyDrawn = Placed + ProxyGap;
	bProxyDrawn = true;
	bProxyWasBased = bBased;
	if (!ProxyGap.IsNearlyZero())
	{
		SetActorLocation(ProxyDrawn, false, nullptr, ETeleportType::TeleportPhysics);
	}
}

void AFMPlayerPawn::HandleRollback(const FMoverTimeStep& CurrentTimeStep, const FMoverTimeStep& ExpungedTimeStep)
{
	++Rollbacks;
	PendingRollbackTo = CurrentTimeStep.ServerFrame;
	PendingRollbackFrom = FMath::Max(PendingRollbackFrom, ExpungedTimeStep.ServerFrame);
}
