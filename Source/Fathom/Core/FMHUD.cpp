#include "Core/FMHUD.h"

#include "Combat/FMCombatComponent.h"
#include "Core/FMPlayerController.h"
#include "Core/FMPlayerState.h"
#include "Deck/FMPlayerPawn.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Net/FMTrace.h"
#include "Ocean/FMOceanSubsystem.h"
#include "Ship/FMShip.h"

void AFMHUD::DrawHUD()
{
	Super::DrawHUD();
	UFont* Font = GEngine->GetMediumFont();
	float Y = 12.0f;
	for (const TPair<FString, FLinearColor>& Line : Lines())
	{
		DrawText(Line.Key, Line.Value, 12.0f, Y, Font);
		Y += 18.0f;
	}
}

TArray<TPair<FString, FLinearColor>> AFMHUD::Lines() const
{
	TArray<TPair<FString, FLinearColor>> Out;
	const FLinearColor White = FLinearColor::White;
	const FLinearColor Grey(0.7f, 0.7f, 0.7f);
	const FLinearColor Yellow = FLinearColor::Yellow;
	const AFMPlayerController* PC = Cast<AFMPlayerController>(GetOwningPlayerController());
	const AFMPlayerPawn* Pawn = PC ? Cast<AFMPlayerPawn>(PC->GetPawn()) : nullptr;
	const AFMPlayerState* Player = PC ? PC->GetPlayerState<AFMPlayerState>() : nullptr;
	const UFMTraceSubsystem* Trace = UFMTraceSubsystem::Get(this);
	const UFMOceanSubsystem* Ocean = UFMOceanSubsystem::Get(this);
	const AFMShip* Ship = AFMShip::Find(GetWorld());

	Out.Emplace(FString::Printf(TEXT("%s  frame %d  lag %d ms  advance %d f"),
		Trace ? *Trace->GetWorldTag() : TEXT("?"), Trace ? Trace->GetFrame() : 0,
		Player ? Player->RoundTripMs : 0, Player ? Player->AdvanceFrames : 0), White);
	if (!Pawn)
	{
		Out.Emplace(TEXT("no pawn"), Yellow);
		return Out;
	}
	FVector Local;
	const bool bOnShip = Pawn->ShipSpaceLocation(Local);
	Out.Emplace(bOnShip
		? FString::Printf(TEXT("%s  on the ship at (%.0f, %.0f, %.0f)"), *Pawn->MovementModeName(), Local.X, Local.Y, Local.Z)
		: FString::Printf(TEXT("%s  off the ship"), *Pawn->MovementModeName()), White);
	if (Ship)
	{
		const FFMStation* Nearest = nullptr;
		float NearestDistance = 0.0f;
		for (const FFMStation& Station : GetDefault<UFMShipSettings>()->Stations)
		{
			const float Distance = Ship->StationDistance(Station, *Pawn);
			if (!Nearest || Distance < NearestDistance)
			{
				Nearest = &Station;
				NearestDistance = Distance;
			}
		}
		if (Nearest)
		{
			Out.Emplace(FString::Printf(TEXT("station %s  %.1f m  %s"), *Nearest->Name.ToString(), NearestDistance / 100.0f,
				NearestDistance <= Nearest->Radius ? TEXT("in radius") : TEXT("out of radius")), White);
		}
		const FFMShipState& S = Ship->GetState();
		const FString Anchor = Ship->GetInputs().bAnchorDown ? TEXT("down")
			: S.AnchorRaise < 1.0f ? FString::Printf(TEXT("raising %.0f%%"), S.AnchorRaise * 100.0f) : TEXT("raised");
		Out.Emplace(FString::Printf(TEXT("ship speed %.0f cm/s  sail %.2f  angle %.0f  rudder %.2f  anchor %s"),
			S.Speed, S.SailLength, S.SailAngle, S.Rudder, *Anchor), White);
	}
	if (Ocean && Ocean->IsReady())
	{
		const FVector2f Wind = Ocean->GetWind();
		Out.Emplace(FString::Printf(TEXT("sea %.1f  wind %.0f deg"), Ocean->GetSeaState(), FMath::RadiansToDegrees(FMath::Atan2(Wind.Y, Wind.X))), White);
	}
	if (const UFMCombatComponent* Combat = Pawn->GetCombat())
	{
		const FFMCombatState* CombatState = Combat->State();
		const EFMCombatPhase Phase = CombatState ? Combat->PhaseAt(*CombatState, FMath::FloorToInt32(Combat->PresentedFrame())) : EFMCombatPhase::Idle;
		const UFMAttackData* Data = CombatState ? Combat->AttackData(CombatState->Attack) : nullptr;
		const bool bInAttack = Phase == EFMCombatPhase::Windup || Phase == EFMCombatPhase::Release || Phase == EFMCombatPhase::Recovery;
		Out.Emplace(FString::Printf(TEXT("combat %s %s  hits taken %d  dealt %d  parries %d"),
			FFMCombatRules::PhaseName(Phase), Data && bInAttack ? *Data->AttackName() : TEXT(""),
			Combat->HitsTaken, Combat->HitsDealt, Combat->ParriesMade), White);
	}
	const FString Notice = PC ? PC->Notice() : FString();
	if (!Notice.IsEmpty())
	{
		Out.Emplace(Notice, Yellow);
	}
	Out.Emplace(TEXT("WASD move  Space jump  LMB horizontal  scroll down overhead  scroll up thrust  RMB parry  Q feint"), Grey);
	Out.Emplace(TEXT("Left/Right wheel  Down unfurls sail, Up furls  [ ] sail angle  X anchor  E ladder  B board  F fly  M mark  console: FM.Latency <ms>"), Grey);
	return Out;
}
