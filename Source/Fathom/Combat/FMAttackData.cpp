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

bool UFMAttackData::BladeAt(int32 Frame, FVector& OutBase, FVector& OutTip) const
{
	if (Frame < 0 || Frame >= FrameCount())
	{
		return false;
	}
	OutBase = BladeBase[Frame];
	OutTip = BladeTip[Frame];
	return true;
}
