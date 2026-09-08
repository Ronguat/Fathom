#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "FMPlayerController.generated.h"

class UInputMappingContext;
struct FInputKeyEventArgs;

/**
 * Owns the key table the pawn reads and the marker hotkey; tells the server its world tag on
 * BeginPlay and carries the client's trace relay. Latches every action key's press until the
 * pawn takes it, which is how the mouse wheel's one-event keys reach a command. The station keys
 * queue station calls on the pawn for its next input command, a value on press and a hold on
 * release: the wheel to its side, Down unfurling the sail and Up furling it, each held on release
 * where the ship has it at that frame; the anchor a
 * toggle, the ladder and the board key a call. A call from outside a station's radius is sent
 * regardless and the refusal the server will make is shown for a moment. Adds
 * DefaultMappingContexts to the local player's Enhanced Input subsystem at BeginPlay, in array order.
 */
UCLASS()
class FATHOM_API AFMPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AFMPlayerController();

	/** Action name to key: move_forward, move_back, move_left, move_right, jump, mark, attack_overhead, attack_horizontal, attack_thrust, parry, feint, wheel_left, wheel_right, sail_up, sail_down, angle_left, angle_right, anchor, ladder, board, fly. */
	UPROPERTY(EditAnywhere, Category="Input")
	TMap<FName, FKey> ActionKeys;

	/** The tag the client reported for its world; empty until its hello arrives. Server only. */
	const FString& GetClientWorldTag() const { return ClientWorldTag; }

	/** A client's trace lines, into the server's session file. */
	UFUNCTION(Server, Reliable)
	void ServerRelayTrace(const TArray<FString>& Lines);

	/** Lands the pawn on the deck at the ladder point, from anywhere. */
	UFUNCTION(Server, Reliable)
	void ServerBoard();

	/** The refusal or notice the HUD shows; empty once its moment has passed. */
	FString Notice() const;

	/** The actions pressed since the last take, then cleared. */
	void TakeLatched(TSet<FName>& OutPressed);

	/** Whether an action's key is down as the key events say, one frame ahead of the polled state; a press already taken counts until its release. */
	bool IsActionDown(FName Action) const;

	/** A key that is never read as down: the mouse wheel. */
	static bool IsMomentary(const FKey& Key);

	using APlayerController::InputKey;
	virtual bool InputKey(const FInputKeyEventArgs& Params) override;

protected:
	UPROPERTY(EditAnywhere, Category="Input")
	TArray<TObjectPtr<UInputMappingContext>> DefaultMappingContexts;

	UFUNCTION(Server, Reliable)
	void ServerHello(const FString& WorldTag);

	virtual void BeginPlay() override;
	virtual void PlayerTick(float DeltaTime) override;

private:
	/** The station keys' edges this tick, into the pawn's next input command. */
	void DriveFromKeys();
	/** A station call from a key: the notice when the pawn stands outside the radius, then the call. */
	void DriveByKey(FName Station, float Value);
	bool WithinStation(FName Station, float& OutDistance) const;
	void Notify(const FString& Text);

	FString ClientWorldTag;
	TSet<FName> Latched;
	TSet<FName> EventDown;
	FString NoticeText;
	double NoticeUntil = 0.0;
};
