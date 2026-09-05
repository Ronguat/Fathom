#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "FMPlayerController.generated.h"

class UInputMappingContext;

/**
 * Owns the key table the pawn reads and the marker hotkey; tells the server its world tag on
 * BeginPlay and carries the client's trace relay. Adds DefaultMappingContexts to the local
 * player's Enhanced Input subsystem at BeginPlay, in array order.
 */
UCLASS()
class FATHOM_API AFMPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AFMPlayerController();

	/** Action name to key: move_forward, move_back, move_left, move_right, jump, mark. */
	UPROPERTY(EditAnywhere, Category="Input")
	TMap<FName, FKey> ActionKeys;

	/** The tag the client reported for its world; empty until its hello arrives. Server only. */
	const FString& GetClientWorldTag() const { return ClientWorldTag; }

	/** A client's trace lines, into the server's session file. */
	UFUNCTION(Server, Reliable)
	void ServerRelayTrace(const TArray<FString>& Lines);

	/** Drives one of the ship's stations: wheel, sail_length, sail_angle, anchor. Sent to the server from the next tick. */
	UFUNCTION(BlueprintCallable, Category="Fathom|Ship")
	void DriveShip(FName Input, float Value);

	UFUNCTION(Server, Reliable)
	void ServerDriveShip(FName Input, float Value);

protected:
	UPROPERTY(EditAnywhere, Category="Input")
	TArray<TObjectPtr<UInputMappingContext>> DefaultMappingContexts;

	UFUNCTION(Server, Reliable)
	void ServerHello(const FString& WorldTag);

	virtual void BeginPlay() override;
	virtual void PlayerTick(float DeltaTime) override;

private:
	FString ClientWorldTag;
	TArray<TPair<FName, float>> PendingDrives;
};
