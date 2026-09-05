#include "FMTimeTools.h"

#include "Misc/App.h"

bool UFMTimeTools::SetFixedTimeStep(bool bEnabled, float DeltaSeconds)
{
	if (bEnabled && DeltaSeconds > 0.0f)
	{
		FApp::SetFixedDeltaTime(static_cast<double>(DeltaSeconds));
	}
	FApp::SetUseFixedTimeStep(bEnabled);
	return FApp::UseFixedTimeStep();
}

bool UFMTimeTools::IsFixedTimeStep()
{
	return FApp::UseFixedTimeStep();
}

float UFMTimeTools::GetFixedDeltaTime()
{
	return static_cast<float>(FApp::GetFixedDeltaTime());
}
