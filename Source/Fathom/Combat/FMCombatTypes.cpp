#include "Combat/FMCombatTypes.h"

#include "HAL/IConsoleManager.h"

namespace
{
	TAutoConsoleVariable<float> CVarAdvanceFraction(TEXT("fm.MeleeAdvanceFraction"), -1.0f,
		TEXT("The fraction of the round trip an attack starts early by; below 0 reads the combat settings"));
	TAutoConsoleVariable<float> CVarAdvanceCapMs(TEXT("fm.MeleeAdvanceCapMs"), -1.0f,
		TEXT("The cap on the advance in ms; below 0 reads the combat settings"));
}

int32 UFMCombatSettings::AdvanceFramesFor(float RoundTripMs, float StepMs) const
{
	const float RequestedFraction = CVarAdvanceFraction.GetValueOnGameThread();
	const float RequestedCap = CVarAdvanceCapMs.GetValueOnGameThread();
	const float Fraction = RequestedFraction >= 0.0f ? RequestedFraction : AdvanceFraction;
	const float Cap = RequestedCap >= 0.0f ? RequestedCap : AdvanceCapMs;
	const float Ms = FMath::Min(Fraction * RoundTripMs, Cap);
	return FMath::Max(0, FMath::RoundToInt(Ms / FMath::Max(StepMs, 1.0f)));
}

// --- inputs -------------------------------------------------------------------------------

FMoverDataStructBase* FFMCombatInputs::Clone() const
{
	return new FFMCombatInputs(*this);
}

bool FFMCombatInputs::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	Super::NetSerialize(Ar, Map, bOutSuccess);
	uint8 TypeByte = static_cast<uint8>(Attack);
	uint8 SideByte = static_cast<uint8>(Side);
	Ar << TypeByte;
	Ar << SideByte;
	Attack = static_cast<EFMAttackType>(TypeByte);
	Side = static_cast<EFMAttackSide>(SideByte);
	Ar.SerializeBits(&bParry, 1);
	Ar.SerializeBits(&bFeint, 1);
	Ar << RenderedFrame;
	Ar << RenderedFraction;
	Ar << AdvanceFrames;
	bOutSuccess = true;
	return true;
}

void FFMCombatInputs::ToString(FAnsiStringBuilderBase& Out) const
{
	Out.Appendf("Attack=%d Side=%d Parry=%d Feint=%d Rendered=%d+%d/255 Advance=%d\n",
		static_cast<int32>(Attack), static_cast<int32>(Side), bParry ? 1 : 0, bFeint ? 1 : 0, RenderedFrame, RenderedFraction, AdvanceFrames);
}

bool FFMCombatInputs::ShouldReconcile(const FMoverDataStructBase& AuthorityState) const
{
	const FFMCombatInputs& Other = static_cast<const FFMCombatInputs&>(AuthorityState);
	return Attack != Other.Attack || Side != Other.Side || bParry != Other.bParry || bFeint != Other.bFeint
		|| RenderedFrame != Other.RenderedFrame || RenderedFraction != Other.RenderedFraction || AdvanceFrames != Other.AdvanceFrames;
}

void FFMCombatInputs::Interpolate(const FMoverDataStructBase& From, const FMoverDataStructBase& To, float Pct)
{
	*this = static_cast<const FFMCombatInputs&>(To);
}

void FFMCombatInputs::Merge(const FMoverDataStructBase& From)
{
	const FFMCombatInputs& Prior = static_cast<const FFMCombatInputs&>(From);
	if (Attack == EFMAttackType::None)
	{
		Attack = Prior.Attack;
		Side = Prior.Side;
	}
	bParry |= Prior.bParry;
	bFeint |= Prior.bFeint;
}

// --- state --------------------------------------------------------------------------------

void FFMCombatState::ClearAttack()
{
	Attack = 0;
	AttackStart = -1;
	HitMask = 0;
	Hits = 0;
	Parried = 0;
}

FMoverDataStructBase* FFMCombatState::Clone() const
{
	return new FFMCombatState(*this);
}

bool FFMCombatState::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	Super::NetSerialize(Ar, Map, bOutSuccess);
	Ar << Attack;
	Ar << AttackStart;
	Ar << ParryStart;
	Ar << HitMask;
	Ar << Hits;
	Ar << Parried;
	bOutSuccess = true;
	return true;
}

void FFMCombatState::ToString(FAnsiStringBuilderBase& Out) const
{
	Out.Appendf("Attack=%d Start=%d Parry=%d Hits=%d Parried=%d\n", Attack, AttackStart, ParryStart, Hits, Parried);
}

bool FFMCombatState::ShouldReconcile(const FMoverDataStructBase& AuthorityState) const
{
	const FFMCombatState& Other = static_cast<const FFMCombatState&>(AuthorityState);
	return Attack != Other.Attack || AttackStart != Other.AttackStart || ParryStart != Other.ParryStart;
}

void FFMCombatState::Interpolate(const FMoverDataStructBase& From, const FMoverDataStructBase& To, float Pct)
{
	*this = static_cast<const FFMCombatState&>(Pct < 0.5f ? From : To);
}

void FFMCombatState::Merge(const FMoverDataStructBase& From)
{
}

// --- rules --------------------------------------------------------------------------------

EFMCombatPhase FFMCombatRules::PhaseAt(const FFMCombatState& State, int32 Frame, const UFMAttackData* Attack, int32 ParryFrames)
{
	if (State.IsAttacking() && Attack)
	{
		const int32 K = Frame - State.AttackStart;
		if (K < Attack->WindupFrames)
		{
			return EFMCombatPhase::Windup;
		}
		if (K < Attack->WindupFrames + Attack->ReleaseFrames)
		{
			return EFMCombatPhase::Release;
		}
		if (K < Attack->TotalFrames)
		{
			return EFMCombatPhase::Recovery;
		}
		return EFMCombatPhase::Idle;
	}
	if (State.ParryStart >= 0 && Frame - State.ParryStart < ParryFrames)
	{
		return EFMCombatPhase::Parry;
	}
	return EFMCombatPhase::Idle;
}

const TCHAR* FFMCombatRules::PhaseName(EFMCombatPhase Phase)
{
	switch (Phase)
	{
	case EFMCombatPhase::Windup: return TEXT("windup");
	case EFMCombatPhase::Release: return TEXT("release");
	case EFMCombatPhase::Recovery: return TEXT("recovery");
	case EFMCombatPhase::Parry: return TEXT("parry");
	default: return TEXT("idle");
	}
}

float FFMCombatRules::SegmentToSegment(const FVector& A0, const FVector& A1, const FVector& B0, const FVector& B1, FVector& OutOnA)
{
	FVector OnB;
	FMath::SegmentDistToSegmentSafe(A0, A1, B0, B1, OutOnA, OnB);
	return static_cast<float>((OutOnA - OnB).Size());
}

float FFMCombatRules::SegmentToPoint(const FVector& A0, const FVector& A1, const FVector& P, FVector& OutOnA)
{
	OutOnA = FMath::ClosestPointOnSegment(P, A0, A1);
	return static_cast<float>((OutOnA - P).Size());
}
