#include "Ship/FMShip.h"

#include "Components/BoxComponent.h"
#include "Components/DynamicMeshComponent.h"
#include "Deck/FMPlayerPawn.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerState.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"
#include "Materials/MaterialInterface.h"
#include "Net/FMTrace.h"
#include "Net/UnrealNetwork.h"
#include "NetworkPredictionWorldManager.h"
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
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
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

	Sail = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Sail"));
	Sail->SetupAttachment(Mesh);
	Sail->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Sail->SetCanEverAffectNavigation(false);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (Cube.Succeeded())
	{
		Sail->SetStaticMesh(Cube.Object);
	}
	if (HullMaterial.Succeeded())
	{
		Sail->SetMaterial(0, HullMaterial.Object);
	}

	Pennant = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pennant"));
	Pennant->SetupAttachment(Mesh);
	Pennant->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Pennant->SetCanEverAffectNavigation(false);
	if (Cube.Succeeded())
	{
		Pennant->SetStaticMesh(Cube.Object);
	}
	if (HullMaterial.Succeeded())
	{
		Pennant->SetMaterial(0, HullMaterial.Object);
	}
	Pennant->SetRelativeScale3D(FVector(3.0, 0.15, 0.4));
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
		AppendStationMarkers(Target, *K);
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

void AFMShip::AppendStationMarkers(UDynamicMesh* Target, const UFMShipSettings& K)
{
	FGeometryScriptPrimitiveOptions Options;
	for (const FFMStation& Station : K.Stations)
	{
		const FVector Deck(Station.Local.X, Station.Local.Y, K.HullHeight * 0.5);
		if (Station.Name == InputWheel)
		{
			UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendCylinder(Target, Options, FTransform(Deck), 12.0f, 110.0f, 12);
			UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendCylinder(Target, Options,
				FTransform(FRotator(90.0f, 0.0f, 0.0f), Deck + FVector(6.0, 0.0, 110.0)), 55.0f, 12.0f, 16);
		}
		else if (Station.Name == InputAnchor)
		{
			UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendBox(Target, Options, FTransform(Deck), 80.0f, 80.0f, 80.0f);
		}
		else if (Station.Name == InputSailLength)
		{
			UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendBox(Target, Options, FTransform(Deck), 120.0f, 120.0f, 30.0f);
		}
		else if (Station.Name == InputLadder)
		{
			const double Rail = FMath::Sign(Station.Local.Y) * (K.HalfWidth - 10.0);
			UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendBox(Target, Options,
				FTransform(FVector(Station.Local.X, Rail, Deck.Z)), 100.0f, 20.0f, 220.0f);
		}
	}
}

float AFMShip::StationDistance(const FFMStation& Station, const AActor& Pawn) const
{
	const FVector Local = GetActorTransform().InverseTransformPosition(Pawn.GetActorLocation());
	return static_cast<float>(FVector2D(Local.X - Station.Local.X, Local.Y - Station.Local.Y).Size());
}

void AFMShip::Land(AFMPlayerPawn& Pawn)
{
	Pawn.HarnessTeleport(GetActorTransform().TransformPosition(GetDefault<UFMShipSettings>()->LadderDeckPoint), State.Heading);
}

void AFMShip::Board(AFMPlayerPawn& Pawn)
{
	if (!HasAuthority())
	{
		return;
	}
	Land(Pawn);
	const APlayerState* Player = Pawn.GetPlayerState();
	FM_TRACE(this, TEXT("BOARD pid=%d sf=%d"), Player ? Player->GetPlayerId() : -1, State.Frame);
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
		const float Distance = Caller ? StationDistance(*Station, *Caller) : 1.0e6f;
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
			Land(*Pawn);
			FM_TRACE(this, TEXT("SHIPIN id=%d sf=%d input=ladder value=%.2f"), ShipId, State.Frame, Value);
		}
		return;
	}
	FFMShipInputs Next = Inputs;
	const bool bHold = Value >= HoldValue;
	float Applied = Value;
	if (Input == InputWheel)
	{
		Next.Wheel = bHold ? State.Rudder : FMath::Clamp(Value, -1.0f, 1.0f);
		Applied = Next.Wheel;
	}
	else if (Input == InputSailLength)
	{
		Next.SailLength = bHold ? State.SailLength : FMath::Clamp(Value, 0.0f, 1.0f);
		Applied = Next.SailLength;
	}
	else if (Input == InputSailAngle)
	{
		Next.SailAngle = bHold ? State.SailAngle : FMath::Clamp(Value, -90.0f, 90.0f);
		Applied = Next.SailAngle;
	}
	else if (Input == InputAnchor)
	{
		Next.bAnchorDown = Value > 0.5f;
		Applied = Next.bAnchorDown ? 1.0f : 0.0f;
	}
	else
	{
		return;
	}
	Next.Frame = State.Frame;
	Inputs = Next;
	RecordInput(Next);
	FM_TRACE(this, TEXT("SHIPIN id=%d sf=%d input=%s value=%.2f"), ShipId, Next.Frame, *Input.ToString(), Applied);
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
	S.AnchorRaise = In.bAnchorDown
		? MoveToward(S.AnchorRaise, 0.0f, Dt / FMath::Max(K.AnchorDropSeconds, 0.01f))
		: MoveToward(S.AnchorRaise, 1.0f, Dt / FMath::Max(K.AnchorRaiseSeconds, 0.01f));

	const FVector2f Wind = Ocean ? Ocean->GetWind() : FVector2f(1.0f, 0.0f);
	const float SailNormalDegrees = (S.Heading + S.SailAngle) * (FMOcean::Pi / 180.0f);
	const FVector2f SailNormal(FMath::Cos(SailNormalDegrees), FMath::Sin(SailNormalDegrees));
	const float Drive = S.SailLength * FMath::Max(K.HeadwindSpeed, Wind.X * SailNormal.X + Wind.Y * SailNormal.Y);
	if (In.bAnchorDown && !S.bAnchorSet && S.AnchorRaise <= 0.0f)
	{
		S.AnchorX = S.X;
		S.AnchorY = S.Y;
		S.bAnchorSet = true;
	}
	else if (!In.bAnchorDown && S.AnchorRaise >= 1.0f)
	{
		S.bAnchorSet = false;
	}
	const float Hold = S.bAnchorSet ? 1.0f - S.AnchorRaise : 0.0f;
	const float HeadingRadians = S.Heading * (FMOcean::Pi / 180.0f);
	const float CosH = FMath::Cos(HeadingRadians);
	const float SinH = FMath::Sin(HeadingRadians);
	float DragNow = K.Drag + Hold * K.AnchorDrag;
	float LinePull = 0.0f;
	if (S.bAnchorSet && Hold > 0.0f)
	{
		const float Dx = S.X - S.AnchorX;
		const float Dy = S.Y - S.AnchorY;
		const float Distance = FMath::Sqrt(Dx * Dx + Dy * Dy);
		if (Distance > K.AnchorLineLength)
		{
			const float Along = (Dx * CosH + Dy * SinH) / Distance;
			LinePull = -K.AnchorLineStiffness * (Distance - K.AnchorLineLength) * Along * Hold;
			DragNow += K.AnchorLineDamping * Hold;
		}
	}
	S.Speed += (K.MaxSpeed * K.Drag * Drive - DragNow * S.Speed + LinePull) * Dt;

	S.Heading = WrapDegrees(S.Heading + S.Rudder * K.TurnRate * (S.Speed / FMath::Max(K.MaxSpeed, 1.0f)) * Dt);
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

void AFMShip::AdvanceTo(int32 Frame)
{
	if (!bHasState || Frame <= State.Frame)
	{
		return;
	}
	Advance(Frame);
	Present();
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
	Advance(CurrentFrame());
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
	if (State.Frame != PresentedFrame)
	{
		LastHullPose = PresentedFrame < 0 ? FTransform(Rotation, Location) : HullPose;
		HullPose = FTransform(Rotation, Location);
		PresentedFrame = State.Frame;
	}
	SetActorLocationAndRotation(Location, Rotation, false, nullptr, ETeleportType::TeleportPhysics);
	Hull->ComponentVelocity = (HullPose.GetLocation() - LastHullPose.GetLocation()) / Dt;
}

FTransform AFMShip::PresentedTransform() const
{
	const UNetworkPredictionWorldManager* Prediction = GetWorld()->GetSubsystem<UNetworkPredictionWorldManager>();
	float Fraction = 1.0f;
	if (Prediction)
	{
		const FFixedTickState& Tick = Prediction->GetFixedTickState();
		Fraction = FMath::Clamp(Tick.UnspentTimeMS / static_cast<float>(FMath::Max(1, Tick.FixedStepMS)), 0.0f, 1.0f);
	}
	return FTransform(FQuat::Slerp(LastHullPose.GetRotation(), HullPose.GetRotation(), Fraction),
		FMath::Lerp(LastHullPose.GetLocation(), HullPose.GetLocation(), Fraction));
}

void AFMShip::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bHasState)
	{
		const UFMShipSettings* K = GetDefault<UFMShipSettings>();
		const float Height = FMath::Max(2.0f, State.SailLength * 900.0f);
		const double Yard = K->HullHeight * 0.5 + 1000.0;
		Sail->SetRelativeLocationAndRotation(FVector(0.0, 0.0, Yard - Height * 0.5), FRotator(0.0f, State.SailAngle, 0.0f));
		Sail->SetRelativeScale3D(FVector(0.2, 6.0, Height / 100.0));
		Mesh->SetWorldTransform(PresentedTransform());
		if (const UFMOceanSubsystem* Ocean = GetWorld()->GetSubsystem<UFMOceanSubsystem>())
		{
			const FVector2f Wind = Ocean->GetWind();
			const float Downwind = FMath::RadiansToDegrees(FMath::Atan2(Wind.Y, Wind.X)) - Mesh->GetComponentRotation().Yaw;
			const FVector Along = FRotator(0.0f, Downwind, 0.0f).Vector() * 150.0;
			Pennant->SetRelativeLocationAndRotation(FVector(Along.X, Along.Y, K->HullHeight * 0.5 + 1220.0), FRotator(0.0f, Downwind, 0.0f));
		}
	}
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
