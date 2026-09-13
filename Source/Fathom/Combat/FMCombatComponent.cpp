#include "Combat/FMCombatComponent.h"

#include "Animation/AnimSequence.h"
#include "Components/PrimitiveComponent.h"
#include "Core/FMPlayerState.h"
#include "DrawDebugHelpers.h"
#include "Engine/NetConnection.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "HAL/IConsoleManager.h"
#include "MoverComponent.h"
#include "MoverDataModelTypes.h"
#include "Net/FMTrace.h"
#include "Net/UnrealNetwork.h"
#include "NetworkPredictionWorldManager.h"
#include "Ship/FMShip.h"

namespace
{
	TAutoConsoleVariable<int32> CVarMeleeDraw(TEXT("fm.MeleeDraw"), 0,
		TEXT("Draws the local pawn's tracer paths, and its drawn weapons' tracers, on every release frame"));
	constexpr double DrawSeconds = 0.5;
}

UFMCombatComponent::UFMCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UFMCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	const UFMCombatSettings* K = GetDefault<UFMCombatSettings>();
	Attacks.SetNum(6);
	for (const TSoftObjectPtr<UFMAttackData>& Soft : K->Attacks)
	{
		if (UFMAttackData* Data = Soft.LoadSynchronous())
		{
			const uint8 Index = AttackIndex(Data->Type, Data->Side);
			if (Index > 0)
			{
				Attacks[Index - 1] = Data;
			}
		}
	}
	FirstPersonParry = K->FirstPersonParryPose.LoadSynchronous();
	ThirdPersonParry = K->ThirdPersonParryPose.LoadSynchronous();
	History.SetNum(FMath::Max(1, K->HistoryFrames));
	Mover = GetOwner()->FindComponentByClass<UMoverComponent>();
	if (Mover)
	{
		Mover->OnPreSimulationTick.AddDynamic(this, &UFMCombatComponent::HandlePreSimulationTick);
		Mover->OnPostMovement.AddDynamic(this, &UFMCombatComponent::HandlePostMovement);
		Mover->OnPostFinalize.AddDynamic(this, &UFMCombatComponent::HandlePostFinalize);
	}
}

void UFMCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UFMCombatComponent, HitsTaken);
	DOREPLIFETIME(UFMCombatComponent, HitsDealt);
	DOREPLIFETIME(UFMCombatComponent, ParriesMade);
}

const UFMAttackData* UFMCombatComponent::AttackData(uint8 Attack) const
{
	return Attack >= 1 && Attack <= Attacks.Num() ? Attacks[Attack - 1].Get() : nullptr;
}

uint8 UFMCombatComponent::AttackIndex(EFMAttackType Type, EFMAttackSide Side) const
{
	if (Type == EFMAttackType::None)
	{
		return 0;
	}
	return static_cast<uint8>((static_cast<int32>(Type) - 1) * 2 + (Side == EFMAttackSide::Right ? 1 : 0) + 1);
}

const FFMCombatState* UFMCombatComponent::State() const
{
	return Mover ? Mover->GetSyncState().SyncStateCollection.FindDataByType<FFMCombatState>() : nullptr;
}

EFMCombatPhase UFMCombatComponent::PhaseAt(const FFMCombatState& S, int32 Frame) const
{
	return FFMCombatRules::PhaseAt(S, Frame, AttackData(S.Attack), GetDefault<UFMCombatSettings>()->ParryFrames);
}

bool UFMCombatComponent::BodyAt(int32 Frame, FFMBodySample& Out) const
{
	if (Frame < 0 || History.Num() == 0)
	{
		return false;
	}
	const FFMBodySample& Sample = History[Frame % History.Num()];
	if (Sample.Frame != Frame)
	{
		return false;
	}
	Out = Sample;
	return true;
}

bool UFMCombatComponent::BodyBetween(int32 Frame, float Fraction, FFMBodySample& Out) const
{
	FFMBodySample Next;
	if (!BodyAt(Frame, Out))
	{
		return false;
	}
	if (Fraction <= 0.0f || !BodyAt(Frame + 1, Next) || Next.Base != Out.Base)
	{
		return true;
	}
	const FFMBodySample& Nearer = Fraction < 0.5f ? Out : Next;
	Out.Location = FMath::Lerp(Out.Location, Next.Location, Fraction);
	Out.Yaw = FMath::Lerp(Out.Yaw, Nearer.Yaw == Out.Yaw ? Out.Yaw : Next.Yaw, Fraction);
	Out.ParryStart = Nearer.ParryStart;
	Out.Attack = Nearer.Attack;
	Out.AttackStart = Nearer.AttackStart;
	return true;
}

bool UFMCombatComponent::Latest(FFMBodySample& Out) const
{
	return BodyAt(LatestFrame, Out);
}

int32 UFMCombatComponent::RenderedFrame(const UWorld* World, float& OutFraction)
{
	OutFraction = 0.0f;
	const UNetworkPredictionWorldManager* Prediction = World ? World->GetSubsystem<UNetworkPredictionWorldManager>() : nullptr;
	if (!Prediction)
	{
		return -1;
	}
	const FFixedTickState::FInterpolationState& Interpolation = Prediction->GetFixedTickState().Interpolation;
	if (Interpolation.ToFrame == INDEX_NONE)
	{
		return -1;
	}
	OutFraction = FMath::Clamp(Interpolation.PCT, 0.0f, 1.0f);
	return Interpolation.ToFrame - 1;
}

double UFMCombatComponent::PresentedFrame() const
{
	const UNetworkPredictionWorldManager* Prediction = GetWorld()->GetSubsystem<UNetworkPredictionWorldManager>();
	if (!Prediction || !Mover)
	{
		return 0.0;
	}
	const FFixedTickState& Tick = Prediction->GetFixedTickState();
	if (GetOwnerRole() == ROLE_SimulatedProxy && Tick.Interpolation.ToFrame != INDEX_NONE)
	{
		return Tick.Interpolation.ToFrame - 1 + Tick.Interpolation.PCT;
	}
	const float Fraction = FMath::Clamp(Tick.UnspentTimeMS / FMath::Max(1.0f, Tick.FixedStepRealTimeMS), 0.0f, 1.0f);
	return Mover->GetLastTimeStep().ServerFrame - 1 + Fraction;
}

bool UFMCombatComponent::Presented(bool bFirstPerson, const UAnimSequence*& OutClip, float& OutTime, float& OutAttackFrame) const
{
	OutAttackFrame = -1.0f;
	const FFMCombatState* S = State();
	if (!S)
	{
		return false;
	}
	const double Frame = PresentedFrame();
	const UNetworkPredictionWorldManager* Prediction = GetWorld()->GetSubsystem<UNetworkPredictionWorldManager>();
	const float StepSeconds = Prediction ? Prediction->GetFixedTickState().FixedStepRealTimeMS / 1000.0f : 1.0f / 60.0f;
	const EFMCombatPhase Phase = PhaseAt(*S, FMath::FloorToInt32(Frame));
	if (Phase == EFMCombatPhase::Windup || Phase == EFMCombatPhase::Release || Phase == EFMCombatPhase::Recovery)
	{
		const UFMAttackData* Data = AttackData(S->Attack);
		OutClip = bFirstPerson ? Data->FirstPerson : Data->ThirdPerson;
		OutAttackFrame = static_cast<float>(FMath::Max(0.0, Frame - S->AttackStart));
		OutTime = OutAttackFrame * StepSeconds;
		return OutClip != nullptr;
	}
	if (Phase == EFMCombatPhase::Parry)
	{
		OutClip = bFirstPerson ? FirstPersonParry : ThirdPersonParry;
		OutTime = 0.0f;
		return OutClip != nullptr;
	}
	return false;
}

FTransform UFMCombatComponent::PresentedPawnFrame() const
{
	const USceneComponent* Visual = Mover ? Mover->GetPrimaryVisualComponent() : nullptr;
	const AActor* Owner = GetOwner();
	const FVector Location = Visual ? Visual->GetComponentLocation() : Owner->GetActorLocation();
	const float Yaw = Visual ? Visual->GetComponentRotation().Yaw : Owner->GetActorRotation().Yaw;
	return FTransform(FRotator(0.0f, Yaw, 0.0f), Location);
}

bool UFMCombatComponent::DrawEnabled()
{
	return CVarMeleeDraw.GetValueOnGameThread() > 0;
}

int32 UFMCombatComponent::PlayerId() const
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	const APlayerState* Player = Pawn ? Pawn->GetPlayerState() : nullptr;
	return Player ? Player->GetPlayerId() : -1;
}

void UFMCombatComponent::TraceScore()
{
	FM_TRACE(this, TEXT("SCORE pid=%d taken=%d dealt=%d parries=%d"), PlayerId(), HitsTaken, HitsDealt, ParriesMade);
}

void UFMCombatComponent::OnRep_Score()
{
	TraceScore();
}

void UFMCombatComponent::ClientDraw_Implementation(const FFMHitDraw& Draw)
{
	PendingDraws.Emplace(GetWorld()->GetTimeSeconds() + DrawSeconds, Draw);
	FM_TRACE(this, TEXT("DRAWRX pid=%d hit=%d rf=%d tracer=%d parried=%d"),
		PlayerId(), Draw.Frame, Draw.RenderedFrame, Draw.Tracer, Draw.bParried ? 1 : 0);
}

void UFMCombatComponent::DrawPending()
{
	UWorld* World = GetWorld();
	const double Now = World->GetTimeSeconds();
	const UFMCombatSettings* K = GetDefault<UFMCombatSettings>();
	const AFMShip* Ship = AFMShip::Find(World);
	const FTransform ShipNow = Ship ? Ship->PresentedTransform() : FTransform::Identity;
	for (int32 Index = PendingDraws.Num() - 1; Index >= 0; --Index)
	{
		if (PendingDraws[Index].Key < Now)
		{
			PendingDraws.RemoveAtSwap(Index);
			continue;
		}
		const FFMHitDraw& Draw = PendingDraws[Index].Value;
		const FTransform& Frame = Draw.bShipSpace ? ShipNow : FTransform::Identity;
		const FColor Colour = Draw.bParried ? FColor::Blue : FColor::Red;
		const FVector Centre = Frame.TransformPosition(Draw.Centre);
		const FVector Up = Frame.TransformVectorNoScale(Draw.Up);
		DrawDebugCapsule(World, Centre, K->BodyHalfHeight, K->BodyRadius, FRotationMatrix::MakeFromZ(Up).ToQuat(), Colour, false, -1.0f, 0, 1.5f);
		DrawDebugSphere(World, Centre + Up * K->HeadHeight, K->HeadRadius, 12, Colour, false, -1.0f, 0, 1.5f);
		DrawDebugLine(World, Frame.TransformPosition(Draw.TracerFrom), Frame.TransformPosition(Draw.TracerTo), Colour, false, -1.0f, 0, 3.0f);
	}
	if (!DrawEnabled())
	{
		return;
	}
	const FFMCombatState* S = State();
	const UFMAttackData* Data = S ? AttackData(S->Attack) : nullptr;
	if (!S || !Data || !S->IsAttacking())
	{
		return;
	}
	const float Kf = static_cast<float>(PresentedFrame() - S->AttackStart);
	TArray<FVector> Tracers, Previous;
	if (Kf < Data->WindupFrames || Kf >= Data->WindupFrames + Data->ReleaseFrames
		|| !Data->TracersBetween(Kf, Tracers) || !Data->TracersBetween(FMath::Max(Kf - 1.0f, 0.0f), Previous))
	{
		return;
	}
	const FTransform Frame = PresentedPawnFrame();
	for (int32 T = 0; T < Tracers.Num(); ++T)
	{
		DrawDebugLine(World, Frame.TransformPosition(Previous[T]), Frame.TransformPosition(Tracers[T]), FColor::Green, false, -1.0f, 0, 1.0f);
	}
	DrawDebugLine(World, Frame.TransformPosition(Tracers[0]), Frame.TransformPosition(Tracers.Last()), FColor::Green, false, -1.0f, 0, 2.5f);
}

void UFMCombatComponent::HandlePreSimulationTick(const FMoverTimeStep& TimeStep, const FMoverInputCmdContext& InputCmd)
{
	const FFMCombatInputs* In = InputCmd.InputCollection.FindDataByType<FFMCombatInputs>();
	FrameInputs = In ? *In : FFMCombatInputs();
	FrameInputsFrame = TimeStep.ServerFrame;
}

void UFMCombatComponent::HandlePostMovement(const FMoverTimeStep& TimeStep, FMoverSyncState& SyncState, FMoverAuxStateContext& AuxState)
{
	FFMCombatState& S = SyncState.SyncStateCollection.FindOrAddMutableDataByType<FFMCombatState>();
	const FMoverDefaultSyncState* Body = SyncState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();
	const int32 Frame = TimeStep.ServerFrame;
	const FFMCombatInputs In = FrameInputsFrame == Frame ? FrameInputs : FFMCombatInputs();
	Transition(S, In, Frame);
	if (GetOwnerRole() != ROLE_Authority || !Body)
	{
		return;
	}
	if (Frame % 60 == 0)
	{
		RefreshAdvance(TimeStep.StepMs);
	}
	if (S.IsAttacking())
	{
		Sweep(S, *Body, Frame, In.RenderedFrame >= 0 ? In.RenderedFrame : Frame - 1, In.RenderedFrame >= 0 ? In.RenderedFraction / 255.0f : 0.0f);
		const UFMAttackData* Data = AttackData(S.Attack);
		if (Data && Frame - S.AttackStart == Data->WindupFrames + Data->ReleaseFrames)
		{
			FM_TRACE(this, TEXT("SWING pid=%d sf=%d attack=%s start=%d hits=%d parried=%d"),
				PlayerId(), Frame, *Data->AttackName(), S.AttackStart, S.Hits, S.Parried);
		}
	}
	Record(*Body, S, Frame);
}

void UFMCombatComponent::RefreshAdvance(float StepMs)
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	AFMPlayerState* Player = Pawn ? Pawn->GetPlayerState<AFMPlayerState>() : nullptr;
	const UNetConnection* Connection = Pawn ? Pawn->GetNetConnection() : nullptr;
	if (!Player || !Connection)
	{
		return;
	}
	const float RoundTripMs = Connection->AvgLag * 1000.0f;
	Player->RoundTripMs = FMath::RoundToInt(RoundTripMs);
	Player->AdvanceFrames = GetDefault<UFMCombatSettings>()->AdvanceFramesFor(RoundTripMs, StepMs);
}

void UFMCombatComponent::Transition(FFMCombatState& S, const FFMCombatInputs& In, int32 Frame)
{
	const UFMCombatSettings* K = GetDefault<UFMCombatSettings>();
	if (S.IsAttacking())
	{
		const UFMAttackData* Data = AttackData(S.Attack);
		const int32 Kf = Frame - S.AttackStart;
		if (!Data || Kf >= Data->TotalFrames || (In.bFeint && Kf < Data->WindupFrames))
		{
			S.ClearAttack();
		}
	}
	if (S.ParryStart >= 0 && Frame - S.ParryStart >= K->ParryFrames)
	{
		S.ParryStart = -1;
	}
	if (S.IsAttacking() || S.ParryStart >= 0)
	{
		return;
	}
	if (In.Attack != EFMAttackType::None && AttackData(AttackIndex(In.Attack, In.Side)))
	{
		uint8 Advance = In.AdvanceFrames;
		if (GetOwnerRole() == ROLE_Authority)
		{
			const APawn* Pawn = Cast<APawn>(GetOwner());
			const AFMPlayerState* Player = Pawn ? Pawn->GetPlayerState<AFMPlayerState>() : nullptr;
			Advance = static_cast<uint8>(FMath::Min<int32>(Advance, Player ? Player->AdvanceFrames : 0));
		}
		S.ClearAttack();
		S.Attack = AttackIndex(In.Attack, In.Side);
		S.AttackStart = FMath::Max(0, Frame - Advance);
	}
	else if (In.bParry)
	{
		S.ParryStart = Frame;
	}
}

void UFMCombatComponent::Record(const FMoverDefaultSyncState& Body, const FFMCombatState& S, int32 Frame)
{
	if (Frame < 0 || History.Num() == 0)
	{
		return;
	}
	FFMBodySample& Sample = History[Frame % History.Num()];
	const UPrimitiveComponent* Base = Body.GetMovementBase();
	Sample.Frame = Frame;
	Sample.Base = Base;
	Sample.Location = Base ? Body.GetLocation_BaseSpace() : Body.GetLocation_WorldSpace();
	Sample.Yaw = Base ? Body.GetOrientation_BaseSpace().Yaw : Body.GetOrientation_WorldSpace().Yaw;
	Sample.ParryStart = S.ParryStart;
	Sample.Attack = S.Attack;
	Sample.AttackStart = S.AttackStart;
	LatestFrame = Frame;
}

void UFMCombatComponent::Sweep(FFMCombatState& S, const FMoverDefaultSyncState& Body, int32 Frame, int32 AtFrame, float AtFraction)
{
	const UFMAttackData* Attack = AttackData(S.Attack);
	const UFMCombatSettings* K = GetDefault<UFMCombatSettings>();
	const int32 Kf = Frame - S.AttackStart;
	if (!Attack || Kf < Attack->WindupFrames || Kf >= Attack->WindupFrames + Attack->ReleaseFrames)
	{
		return;
	}
	TConstArrayView<FVector> Tracers, Previous;
	if (!Attack->TracersAt(Kf, Tracers))
	{
		return;
	}
	if (!Attack->TracersAt(Kf - 1, Previous))
	{
		Previous = Tracers;
	}

	const UPrimitiveComponent* MyBase = Body.GetMovementBase();
	const FTransform BaseTransform = MyBase ? MyBase->GetComponentTransform() : FTransform::Identity;
	const FVector MyLocation = MyBase ? Body.GetLocation_BaseSpace() : Body.GetLocation_WorldSpace();
	const float MyYaw = MyBase ? Body.GetOrientation_BaseSpace().Yaw : Body.GetOrientation_WorldSpace().Yaw;
	const FTransform Pawn(FRotator(0.0f, MyYaw, 0.0f), MyLocation);
	const FVector Up = MyBase ? BaseTransform.InverseTransformVectorNoScale(FVector::UpVector) : FVector::UpVector;
	const float AxisHalf = FMath::Max(0.0f, K->BodyHalfHeight - K->BodyRadius);
	const int32 MyId = PlayerId();

	for (TActorIterator<APawn> It(GetWorld()); It; ++It)
	{
		UFMCombatComponent* Other = It->FindComponentByClass<UFMCombatComponent>();
		if (!Other || Other == this)
		{
			continue;
		}
		const int32 TheirId = Other->PlayerId();
		const uint16 Bit = static_cast<uint16>(1u << (FMath::Max(TheirId, 0) % 16));
		if (S.HitMask & Bit)
		{
			continue;
		}
		FFMBodySample Their;
		if (!Other->BodyBetween(AtFrame, AtFraction, Their))
		{
			continue;
		}
		const bool bSameBase = Their.Base.Get() == MyBase;
		FVector Centre = Their.Location;
		if (!bSameBase)
		{
			const FVector World = Their.Base.IsValid() ? Their.Base->GetComponentTransform().TransformPosition(Their.Location) : Their.Location;
			Centre = MyBase ? BaseTransform.InverseTransformPosition(World) : World;
		}
		const FVector Head = Centre + Up * K->HeadHeight;
		const FVector Axis0 = Centre - Up * AxisHalf;
		const FVector Axis1 = Centre + Up * AxisHalf;
		for (int32 T = 0; T < Tracers.Num(); ++T)
		{
			const FVector A0 = Pawn.TransformPosition(Previous[T]);
			const FVector A1 = Pawn.TransformPosition(Tracers[T]);
			FVector Contact;
			const TCHAR* Part = nullptr;
			if (FFMCombatRules::SegmentToPoint(A0, A1, Head, Contact) <= K->HeadRadius)
			{
				Part = TEXT("head");
			}
			else if (FFMCombatRules::SegmentToSegment(A0, A1, Axis0, Axis1, Contact) <= K->BodyRadius)
			{
				Part = TEXT("body");
			}
			if (!Part)
			{
				continue;
			}
			S.HitMask |= Bit;
			FFMBodySample Now;
			const float Moved = Other->Latest(Now) && Now.Base == Their.Base ? static_cast<float>((Now.Location - Their.Location).Size()) : -1.0f;
			const bool bWindow = Their.ParryStart >= 0 && AtFrame - Their.ParryStart < K->ParryFrames;
			const FVector Forward = FRotator(0.0f, Their.Yaw, 0.0f).Vector();
			const FVector ToMe = (MyLocation - Centre).GetSafeNormal2D();
			const bool bFacing = FVector::DotProduct(Forward, ToMe) >= FMath::Cos(FMath::DegreesToRadians(K->ParryConeDegrees * 0.5f));
			FFMHitDraw Draw;
			Draw.TracerFrom = A0;
			Draw.TracerTo = A1;
			Draw.Centre = Centre;
			Draw.Up = Up;
			Draw.Frame = Frame;
			Draw.RenderedFrame = AtFrame;
			Draw.Tracer = static_cast<uint8>(T);
			Draw.bShipSpace = MyBase != nullptr;
			Draw.bParried = bWindow && bFacing;
			ClientDraw(Draw);
			Other->ClientDraw(Draw);
			if (bWindow && bFacing)
			{
				++S.Parried;
				++Other->ParriesMade;
				Other->TraceScore();
				FM_TRACE(this, TEXT("PARRY pid=%d sf=%d target=%d rf=%d rp=%.2f k=%d tracer=%d margin=%d x=%.1f y=%.1f z=%.1f"),
					MyId, Frame, TheirId, AtFrame, AtFraction, Kf, T, K->ParryFrames - (AtFrame - Their.ParryStart), Contact.X, Contact.Y, Contact.Z);
			}
			else
			{
				++S.Hits;
				++HitsDealt;
				++Other->HitsTaken;
				TraceScore();
				Other->TraceScore();
				FM_TRACE(this, TEXT("HIT pid=%d sf=%d target=%d rf=%d rp=%.2f k=%d tracer=%d part=%s x=%.1f y=%.1f z=%.1f tx=%.1f ty=%.1f tz=%.1f moved=%.1f window=%d facing=%d"),
					MyId, Frame, TheirId, AtFrame, AtFraction, Kf, T, Part, Contact.X, Contact.Y, Contact.Z, Centre.X, Centre.Y, Centre.Z, Moved, bWindow ? 1 : 0, bFacing ? 1 : 0);
			}
			break;
		}
	}
}

void UFMCombatComponent::HandlePostFinalize(const FMoverSyncState& SyncState, const FMoverAuxStateContext& AuxState)
{
	const FFMCombatState* S = SyncState.SyncStateCollection.FindDataByType<FFMCombatState>();
	if (!S || !Mover)
	{
		return;
	}
	const int32 Frame = Mover->GetLastTimeStep().ServerFrame;
	const EFMCombatPhase Phase = PhaseAt(*S, Frame);
	if (Phase == TracedPhase && S->Attack == TracedAttack && S->AttackStart == TracedStart && S->ParryStart == TracedParry)
	{
		return;
	}
	TracedPhase = Phase;
	TracedAttack = S->Attack;
	TracedStart = S->AttackStart;
	TracedParry = S->ParryStart;
	const UFMAttackData* Data = AttackData(S->Attack);
	FM_TRACE(this, TEXT("COMBAT pid=%d sf=%d phase=%s attack=%s start=%d parry=%d"),
		PlayerId(), Frame, FFMCombatRules::PhaseName(Phase), Data ? *Data->AttackName() : TEXT("-"), S->AttackStart, S->ParryStart);
}
