#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "FMHUD.generated.h"

/**
 * The lines a human at the play window reads: the world tag, frame, measured lag and advance;
 * the pawn's mode and ship-space place; the nearest station and whether it is in radius; the
 * ship's speed and station values; sea state and wind; the combat phase and the tallies; the
 * controller's notice; the key legend. The loop reads none of it.
 */
UCLASS()
class FATHOM_API AFMHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

private:
	TArray<TPair<FString, FLinearColor>> Lines() const;
};
