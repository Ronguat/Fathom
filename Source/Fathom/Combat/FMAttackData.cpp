#include "Combat/FMAttackData.h"

FString UFMAttackData::AttackName() const
{
	const TCHAR* TypeName = TEXT("none");
	switch (Type)
	{
	case EFMAttackType::Overhead: TypeName = TEXT("overhead"); break;
	case EFMAttackType::Horizontal: TypeName = TEXT("horizontal"); break;
	case EFMAttackType::Thrust: TypeName = TEXT("thrust"); break;
	default: break;
	}
	return FString::Printf(TEXT("%s_%s"), TypeName, Side == EFMAttackSide::Left ? TEXT("l") : TEXT("r"));
}

bool UFMAttackData::TracersAt(int32 Frame, TConstArrayView<FVector>& Out) const
{
	if (Frame < 0 || Frame >= FrameCount())
	{
		return false;
	}
	Out = TConstArrayView<FVector>(Tracers.GetData() + Frame * TracerCount, TracerCount);
	return true;
}

bool UFMAttackData::TracersBetween(float Frame, TArray<FVector>& Out) const
{
	const int32 Frames = FrameCount();
	if (Frame < 0.0f || Frames == 0)
	{
		return false;
	}
	const int32 A = FMath::Min(FMath::FloorToInt32(Frame), Frames - 1);
	const int32 B = FMath::Min(A + 1, Frames - 1);
	const float Alpha = FMath::Clamp(Frame - A, 0.0f, 1.0f);
	Out.SetNumUninitialized(TracerCount);
	for (int32 T = 0; T < TracerCount; ++T)
	{
		Out[T] = FMath::Lerp(Tracers[A * TracerCount + T], Tracers[B * TracerCount + T], Alpha);
	}
	return true;
}
