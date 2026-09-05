#include "Ship/FMShip.h"

#include "Components/BoxComponent.h"
#include "Components/DynamicMeshComponent.h"
#include "Deck/FMPlayerPawn.h"
#include "EngineUtils.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"
#include "Materials/MaterialInterface.h"
#include "Net/FMTrace.h"
#include "Net/UnrealNetwork.h"
#include "Ocean/FMOcean.h"
#include "Ocean/FMOceanSubsystem.h"
#include "UDynamicMesh.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	const FName InputWheel(TEXT("wheel"));
	const FName InputSailLength(TEXT("sail_length"));
	const FName InputSailAngle(TEXT("sail_angle"));
	const FName InputAnchor(TEXT("anchor"));
	const FName InputLadder(TEXT("ladder"));

	float MoveToward(float Value, float Target, float MaxDelta)
	{
		return Value + FMath::Clamp(Target - Value, -MaxDelta, MaxDelta);
	}

	float WrapDegrees(float Degrees)
	{
		float Turns = (Degrees + 180.0f) / 360.0f;
		return (Turns - FMath::FloorToFloat(Turns)) * 360.0f - 180.0f;
	}
}

AFMShip::AFMShip()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicatingMovement(false);
	SetNetUpdateFrequency(30.0f);

	Hull = CreateDefaultSubobject<UBoxComponent>(TEXT("Hull"));
	Hull->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	Hull->SetCanEverAffectNavigation(false);
	RootComponent = Hull;

	Mesh = CreateDefaultSubobject<UDynamicMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Hull);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetCanEverAffectNavigation(false);
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> HullMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (HullMaterial.Succeeded())
	{
		Mesh->SetMaterial(0, HullMaterial.Object);
	}
}

AFMShip* AFMShip::Find(const UWorld* World)
{
	for (TActorIterator<AFMShip> It(World); It; ++It)
	{
		return *It;
	}
	return nullptr;
}

void AFMShip::BeginPlay()
{
	Super::BeginPlay();
	TickStartHandle = FWorldDelegates::OnWorldTickStart.AddUObject(this, &AFMShip::OnWorldTickStart);
	const UFMShipSettings* K = GetDefault<UFMShipSettings>();
	Hull->SetBoxExtent(FVector(K->HalfLength, K->HalfWidth, K->HullHeight * 0.5f));
	if (UDynamicMesh* Target = Mesh->GetDynamicMesh())
	{
		Target->Reset();
		FGeometryScriptPrimitiveOptions Options;
		UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendBox(Target, Options,
			FTransform(FVector(0.0, 0.0, -K->HullHeight * 0.5)), K->HalfLength * 2.0f, K->HalfWidth * 2.0f, K->HullHeight);
		UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendBox(Target, Options,
			FTransform(FVector(0.0, 0.0, K->HullHeight * 0.5)), 60.0f, 60.0f, 1200.0f);
		Mesh->NotifyMeshUpdated();
	}
	if (HasAuthority())
	{
		State = FFMShipState();
		State.Frame = CurrentFrame();
		State.X = static_cast<float>(K->SpawnXY.X);
		State.Y = static_cast<float>(K->SpawnXY.Y);
		State.Heading = K->SpawnHeading;
		Inputs = FFMShipInputs();
		Inputs.Frame = State.Frame;
		History.Add(Inputs);
		Snapshot = State;
		LastSnapshotFrame = State.Frame;
		bHasState = true;
		Present();
	}
}

int32 AFMShip::CurrentFrame() const
{
	const UFMTraceSubsystem* TraceSubsystem = GetWorld()->GetSubsystem<UFMTraceSubsystem>();
	return TraceSubsystem ? TraceSubsystem->GetFrame() : 0;
}

void AFMShip::Apply(FName Input, float Value, AActor* Caller)
{
	if (!HasAuthority())
	{
		return;
	}
	const UFMShipSettings* K = GetDefault<UFMShipSettings>();
	const FFMStation* Station = K->Stations.FindByPredicate([&](const FFMStation& S) { return S.Name == Input; });
	if (Station)
	{
		const FVector Local = Caller ? GetActorTransform().InverseTransformPosition(Caller->GetActorLocation()) : FVector(1.0e6, 1.0e6, 0.0);
		const float Distance = FVector2D(Local.X - Station->Local.X, Local.Y - Station->Local.Y).Size();
		if (Distance > Station->Radius)
		{
			FM_TRACE(this, TEXT("SHIPNO id=%d sf=%d input=%s dist=%.0f"), ShipId, State.Frame, *Input.ToString(), Distance);
			return;
		}
	}
	if (Input == InputLadder)
	{
		if (AFMPlayerPawn* Pawn = Cast<AFMPlayerPawn>(Caller))
		{
			Pawn->HarnessTeleport(GetActorTransform().TransformPosition(K->LadderDeckPoint), State.Heading);
			FM_TRACE(this, TEXT("SHIPIN id=%d sf=%d input=ladder value=%.2f"), ShipId, State.Frame, Value);
		}
		return;
	}
	FFMShipInputs Next = Inputs;
	if (Input == InputWheel)
	{
		Next.Wheel = FMath::Clamp(Value, -1.0f, 1.0f);
	}
	else if (Input == InputSailLength)
	{
		Next.SailLength = FMath::Clamp(Value, 0.0f, 1.0f);
	}
	else if (Input == InputSailAngle)
	{
		Next.SailAngle = FMath::Clamp(Value, -90.0f, 90.0f);
	}
	else if (Input == InputAnchor)
	{
		Next.bAnchorDown = Value > 0.5f;
	}
	else
	{
		return;
	}
	Next.Frame = State.Frame;
	Inputs = Next;
	RecordInput(Next);
	FM_TRACE(this, TEXT("SHIPIN id=%d sf=%d input=%s value=%.2f"), ShipId, Next.Frame, *Input.ToString(), Value);
}

void AFMShip::RecordInput(const FFMShipInputs& In)
{
	int32 Index = 0;
	while (Index < History.Num() && History[Index].Frame <= In.Frame)
	{
		++Index;
	}
	if (Index > 0 && History[Index - 1].Frame == In.Frame)
	{
		History[Index - 1] = In;
	}
	else
	{
		History.Insert(In, Index);
	}
}

const FFMShipInputs& AFMShipInputsFallback()
{
	static const FFMShipInputs None;
	return None;
}

const FFMShipInputs& AFMShip::InputsAt(int32 Frame) const
{
	const FFMShipInputs* Best = nullptr;
	for (const FFMShipInputs& In : History)
	{
		if (In.Frame <= Frame)
		{
			Best = &In;
		}
	}
	return Best ? *Best : (History.Num() ? History[0] : AFMShipInputsFallback());
}

void AFMShip::Step(FFMShipState& S, const FFMShipInputs& In, const UFMShipSettings& K, const UFMOceanSubsystem* Ocean, float Dt)
{
	S.SailLength = MoveToward(S.SailLength, In.SailLength, K.SailRate * Dt);
	S.SailAngle = MoveToward(S.SailAngle, In.SailAngle, K.SailAngleRate * Dt);
	S.Rudder = MoveToward(S.Rudder, In.Wheel, K.RudderRate * Dt);
	S.AnchorRaise = In.bAnchorDown ? 0.0f : MoveToward(S.AnchorRaise, 1.0f, Dt / FMath::Max(K.AnchorRaiseSeconds, 0.01f));

	const FVector2f Wind = Ocean ? Ocean->GetWind() : FVector2f(1.0f, 0.0f);
	const float SailNormalDegrees = (S.Heading + S.SailAngle) * (FMOcean::Pi / 180.0f);
	const FVector2f SailNormal(FMath::Cos(SailNormalDegrees), FMath::Sin(SailNormalDegrees));
	const float Drive = S.SailLength * FMath::Max(0.0f, Wind.X * SailNormal.X + Wind.Y * SailNormal.Y);
	const float DragNow = K.Drag + (1.0f - S.AnchorRaise) * K.AnchorDrag;
	S.Speed += (K.MaxSpeed * K.Drag * Drive - DragNow * S.Speed) * Dt;

	S.Heading = WrapDegrees(S.Heading + S.Rudder * K.TurnRate * (S.Speed / FMath::Max(K.MaxSpeed, 1.0f)) * Dt);
	const float HeadingRadians = S.Heading * (FMOcean::Pi / 180.0f);
	const float CosH = FMath::Cos(HeadingRadians);
	const float SinH = FMath::Sin(HeadingRadians);
	S.X += S.Speed * CosH * Dt;
	S.Y += S.Speed * SinH * Dt;

	const int32 NextFrame = S.Frame + 1;
	const auto HeightAtLocal = [&](float LocalX, float LocalY)
	{
		const FVector2f World(S.X + LocalX * CosH - LocalY * SinH, S.Y + LocalX * SinH + LocalY * CosH);
		return Ocean ? Ocean->HeightAt(World, NextFrame) : 0.0f;
	};
	const float Bow = HeightAtLocal(K.HalfLength, 0.0f);
	const float Stern = HeightAtLocal(-K.HalfLength, 0.0f);
	const float Starboard = HeightAtLocal(0.0f, K.HalfWidth);
	const float Port = HeightAtLocal(0.0f, -K.HalfWidth);
	const float TargetHeave = (Bow + Stern + Starboard + Port) * 0.25f;
	const float TargetPitch = FMath::Atan2(Bow - Stern, 2.0f * K.HalfLength) * (180.0f / FMOcean::Pi);
	const float TargetRoll = FMath::Atan2(Port - Starboard, 2.0f * K.HalfWidth) * (180.0f / FMOcean::Pi);

	S.HeaveVel += (K.FitStiffness * (TargetHeave - S.Heave) - K.FitDamping * S.HeaveVel) * Dt;
	S.Heave += S.HeaveVel * Dt;
	S.PitchVel += (K.FitStiffness * (TargetPitch - S.Pitch) - K.FitDamping * S.PitchVel) * Dt;
	S.Pitch += S.PitchVel * Dt;
	S.RollVel += (K.FitStiffness * (TargetRoll - S.Roll) - K.FitDamping * S.RollVel) * Dt;
	S.Roll += S.RollVel * Dt;
	S.Frame = NextFrame;
}

void AFMShip::Advance(int32 ToFrame)
{
	const UFMShipSettings* K = GetDefault<UFMShipSettings>();
	const UFMOceanSubsystem* Ocean = GetWorld()->GetSubsystem<UFMOceanSubsystem>();
	const float Dt = Ocean ? Ocean->TimeOfFrame(1) : 1.0f / 60.0f;
	int32 Guard = 600;
	while (State.Frame < ToFrame && Guard-- > 0)
	{
		Step(State, InputsAt(State.Frame), *K, Ocean, Dt);
	}
}

void AFMShip::Reintegrate(int32 ToFrame)
{
	State = Snapshot;
	Advance(ToFrame);
}

void AFMShip::EndPlay(const EEndPlayReason::Type Reason)
{
	FWorldDelegates::OnWorldTickStart.Remove(TickStartHandle);
	Super::EndPlay(Reason);
}

void AFMShip::OnWorldTickStart(UWorld* World, ELevelTick TickType, float DeltaSeconds)
{
	if (World != GetWorld() || !bHasState)
	{
		return;
	}
	Advance(CurrentFrame() + 1);
	if (HasAuthority())
	{
		const UFMShipSettings* K = GetDefault<UFMShipSettings>();
		if (State.Frame - LastSnapshotFrame >= FMath::Max(1, K->SnapshotEveryFrames))
		{
			Snapshot = State;
			LastSnapshotFrame = State.Frame;
			while (History.Num() > 1 && History[1].Frame <= Snapshot.Frame)
			{
				History.RemoveAt(0);
			}
		}
	}
	Present();
	Trace();
}

void AFMShip::Present()
{
	const UFMShipSettings* K = GetDefault<UFMShipSettings>();
	const UFMOceanSettings* OceanSettings = GetDefault<UFMOceanSettings>();
	const UFMOceanSubsystem* Ocean = GetWorld()->GetSubsystem<UFMOceanSubsystem>();
	const float Dt = Ocean ? Ocean->TimeOfFrame(1) : 1.0f / 60.0f;
	const FVector Location(State.X, State.Y, OceanSettings->PlaneZ + State.Heave + K->HullCenterAboveWater);
	const FRotator Rotation(State.Pitch, State.Heading, State.Roll);
	SetActorLocationAndRotation(Location, Rotation, false, nullptr, ETeleportType::TeleportPhysics);
	Hull->ComponentVelocity = (Location - LastLocation) / Dt;
	LastLocation = Location;
}

void AFMShip::Trace()
{
	const UFMShipSettings* K = GetDefault<UFMShipSettings>();
	const int32 Every = FMath::Max(1, K->TraceEveryFrames);
	if (State.Frame % Every != 0 || State.Frame == LastTraceFrame)
	{
		return;
	}
	LastTraceFrame = State.Frame;
	FM_TRACE(this, TEXT("SHIP id=%d sf=%d x=%.2f y=%.2f z=%.2f yaw=%.2f pitch=%.2f roll=%.2f speed=%.2f sail=%.2f angle=%.1f rudder=%.2f anchor=%.2f"),
		ShipId, State.Frame, State.X, State.Y, State.Heave, State.Heading, State.Pitch, State.Roll,
		State.Speed, State.SailLength, State.SailAngle, State.Rudder, State.AnchorRaise);
}

void AFMShip::OnRep_Snapshot()
{
	bHasState = true;
	LastSnapshotFrame = Snapshot.Frame;
	while (History.Num() > 1 && History[1].Frame <= Snapshot.Frame)
	{
		History.RemoveAt(0);
	}
	Reintegrate(FMath::Max(CurrentFrame(), Snapshot.Frame));
}

void AFMShip::OnRep_Inputs()
{
	RecordInput(Inputs);
	if (bHasState && Inputs.Frame < State.Frame)
	{
		Reintegrate(State.Frame);
	}
}

void AFMShip::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFMShip, Snapshot);
	DOREPLIFETIME(AFMShip, Inputs);
	DOREPLIFETIME(AFMShip, ShipId);
}
