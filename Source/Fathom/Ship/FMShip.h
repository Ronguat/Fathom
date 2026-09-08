#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameFramework/Actor.h"
#include "FMShip.generated.h"

class AFMPlayerPawn;
class UBoxComponent;
class UDynamicMesh;
class UDynamicMeshComponent;
class UFMOceanSubsystem;

/** A station: a named input, its place in ship space, and how near a pawn must stand to drive it. */
USTRUCT()
struct FFMStation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Station")
	FName Name;

	UPROPERTY(EditAnywhere, Category="Station")
	FVector2D Local = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, Category="Station")
	float Radius = 300.0f;
};

/** The ship's knobs; Config/DefaultGame.ini is their home. Speeds in cm/s, rates per second, angles in degrees. */
UCLASS(config=Game, defaultconfig, meta=(DisplayName="Fathom Ship"))
class FATHOM_API UFMShipSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(config, EditAnywhere, Category="Stations") TArray<FFMStation> Stations;
	UPROPERTY(config, EditAnywhere, Category="Stations") FVector LadderDeckPoint = FVector(0.0, 250.0, 270.0);

	UPROPERTY(config, EditAnywhere, Category="Sailing") float MaxSpeed = 1000.0f;
	UPROPERTY(config, EditAnywhere, Category="Sailing") float Drag = 0.3f;
	/** The fraction of full drive a sail makes head to wind, so a ship under sail can always turn. */
	UPROPERTY(config, EditAnywhere, Category="Sailing") float HeadwindSpeed = 0.2f;
	/** Drag added once the anchor lies on the bottom, slack line or taut. */
	UPROPERTY(config, EditAnywhere, Category="Sailing") float AnchorDrag = 0.3f;
	/** The anchor's fall to the bottom after the drop, during which nothing acts on the ship. */
	UPROPERTY(config, EditAnywhere, Category="Sailing") float AnchorDropSeconds = 2.0f;
	UPROPERTY(config, EditAnywhere, Category="Sailing") float AnchorRaiseSeconds = 8.0f;
	/** The anchor line: the ship runs this far from where the anchor bit before the line catches, then a spring of this stiffness and damping, per second squared and per second, holds it there. */
	UPROPERTY(config, EditAnywhere, Category="Sailing") float AnchorLineLength = 800.0f;
	UPROPERTY(config, EditAnywhere, Category="Sailing") float AnchorLineStiffness = 9.0f;
	UPROPERTY(config, EditAnywhere, Category="Sailing") float AnchorLineDamping = 2.4f;
	UPROPERTY(config, EditAnywhere, Category="Sailing") float SailRate = 0.5f;
	UPROPERTY(config, EditAnywhere, Category="Sailing") float SailAngleRate = 30.0f;
	UPROPERTY(config, EditAnywhere, Category="Sailing") float RudderRate = 1.0f;
	UPROPERTY(config, EditAnywhere, Category="Sailing") float TurnRate = 15.0f;

	UPROPERTY(config, EditAnywhere, Category="Hull") float HalfLength = 1200.0f;
	UPROPERTY(config, EditAnywhere, Category="Hull") float HalfWidth = 400.0f;
	UPROPERTY(config, EditAnywhere, Category="Hull") float HullHeight = 300.0f;
	UPROPERTY(config, EditAnywhere, Category="Hull") float HullCenterAboveWater = 100.0f;
	/** How far past the hull's extent a pawn in the air still counts as over the deck, keeping the hull as its base. */
	UPROPERTY(config, EditAnywhere, Category="Hull") float DeckMargin = 50.0f;
	UPROPERTY(config, EditAnywhere, Category="Hull") float FitStiffness = 6.0f;
	UPROPERTY(config, EditAnywhere, Category="Hull") float FitDamping = 4.0f;

	UPROPERTY(config, EditAnywhere, Category="Net") int32 SnapshotEveryFrames = 12;
	UPROPERTY(config, EditAnywhere, Category="Net") int32 TraceEveryFrames = 6;

	UPROPERTY(config, EditAnywhere, Category="Spawn") FVector2D SpawnXY = FVector2D(0.0, 15000.0);
	UPROPERTY(config, EditAnywhere, Category="Spawn") float SpawnHeading = 0.0f;
};

/** The station targets, with the frame they took effect. */
USTRUCT()
struct FFMShipInputs
{
	GENERATED_BODY()

	UPROPERTY() float SailLength = 0.0f;
	UPROPERTY() float SailAngle = 0.0f;
	UPROPERTY() float Wheel = 0.0f;
	UPROPERTY() bool bAnchorDown = false;
	UPROPERTY() int32 Frame = 0;
};

/** The compact state at a frame, from which any world integrates forward. */
USTRUCT()
struct FFMShipState
{
	GENERATED_BODY()

	UPROPERTY() int32 Frame = 0;
	UPROPERTY() float X = 0.0f;
	UPROPERTY() float Y = 0.0f;
	UPROPERTY() float Heading = 0.0f;
	UPROPERTY() float Speed = 0.0f;
	UPROPERTY() float SailLength = 0.0f;
	UPROPERTY() float SailAngle = 0.0f;
	UPROPERTY() float Rudder = 0.0f;
	UPROPERTY() float AnchorRaise = 1.0f;
	/** Where the anchor lies once it has reached the bottom: the ship's position at that frame. */
	UPROPERTY() float AnchorX = 0.0f;
	UPROPERTY() float AnchorY = 0.0f;
	UPROPERTY() bool bAnchorSet = false;
	UPROPERTY() float Heave = 0.0f;
	UPROPERTY() float HeaveVel = 0.0f;
	UPROPERTY() float Roll = 0.0f;
	UPROPERTY() float RollVel = 0.0f;
	UPROPERTY() float Pitch = 0.0f;
	UPROPERTY() float PitchVel = 0.0f;
};

/**
 * A deterministic kinematic ship. Every world steps the same integrator one frame at a time from
 * a replicated snapshot through the replicated input history to its own frame; the server's
 * state is the truth, a client re-integrates when a snapshot or an input arrives late. Surge
 * comes from the sail against the wind with a floor head to wind; a dropped anchor lies where
 * the ship was, and the ship runs to the end of its line, catches, and is held there. The hull
 * box is moved by transform and publishes its velocity. Stations are the named inputs wheel,
 * sail_length, sail_angle and anchor, each with a placeholder on the deck. Writes SHIP every
 * TraceEveryFrames, and SHIPIN, SHIPNO and BOARD on the server.
 */
UCLASS()
class FATHOM_API AFMShip : public AActor
{
	GENERATED_BODY()

public:
	AFMShip();

	static AFMShip* Find(const UWorld* World);

	/** A station value meaning "hold where the server has it now": a key's release under latency, before the press's effect has come back to the client. */
	static constexpr float HoldValue = 1000.0f;

	/** Applies a station input on the server at the current frame, if the caller stands within the station's radius; HoldValue takes the station's current position. */
	void Apply(FName Input, float Value, AActor* Caller);

	/** Lands a pawn on the deck at the ladder point through its simulation. Server only. */
	void Board(AFMPlayerPawn& Pawn);

	/** Steps the ship to the frame a pawn is about to simulate and presents it there, from inside the fixed tick: the hull moves only when the simulation does, once per fixed step however many a world tick runs. */
	void AdvanceTo(int32 Frame);

	/** A pawn's distance from a station along the deck, in ship space. */
	float StationDistance(const FFMStation& Station, const AActor& Pawn) const;

	/** The hull's pose between its previous frame and its current one, at the prediction framework's leftover fraction of a step. */
	FTransform PresentedTransform() const;

	const FFMShipState& GetState() const { return State; }
	/** The station targets the server last applied. */
	const FFMShipInputs& GetInputs() const { return Inputs; }

	static void Step(FFMShipState& S, const FFMShipInputs& In, const UFMShipSettings& K, const UFMOceanSubsystem* Ocean, float Dt);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UFUNCTION()
	void OnRep_Snapshot();

	UFUNCTION()
	void OnRep_Inputs();

	UPROPERTY(ReplicatedUsing=OnRep_Snapshot)
	FFMShipState Snapshot;

	UPROPERTY(ReplicatedUsing=OnRep_Inputs)
	FFMShipInputs Inputs;

	UPROPERTY(Replicated)
	int32 ShipId = 1;

	UPROPERTY(VisibleAnywhere, Category="Fathom")
	TObjectPtr<UBoxComponent> Hull;

	UPROPERTY(VisibleAnywhere, Category="Fathom")
	TObjectPtr<UDynamicMeshComponent> Mesh;

	/** A slab hanging from the mast's yard, unfurled downward as far as the sail is set and turned to its angle. */
	UPROPERTY(VisibleAnywhere, Category="Fathom")
	TObjectPtr<UStaticMeshComponent> Sail;

	/** A pennant at the masthead streaming the way the wind blows. */
	UPROPERTY(VisibleAnywhere, Category="Fathom")
	TObjectPtr<UStaticMeshComponent> Pennant;

private:
	void OnWorldTickStart(UWorld* World, ELevelTick TickType, float DeltaSeconds);
	void Land(AFMPlayerPawn& Pawn);
	void AppendStationMarkers(UDynamicMesh* Target, const UFMShipSettings& K);
	int32 CurrentFrame() const;
	const FFMShipInputs& InputsAt(int32 Frame) const;
	void RecordInput(const FFMShipInputs& In);
	void Reintegrate(int32 ToFrame);
	void Advance(int32 ToFrame);
	void Present();
	void Trace();

	FFMShipState State;
	TArray<FFMShipInputs> History;
	FTransform HullPose;
	FTransform LastHullPose;
	int32 PresentedFrame = -1;
	FDelegateHandle TickStartHandle;
	bool bHasState = false;
	int32 LastTraceFrame = -1;
	int32 LastSnapshotFrame = -1;
};
