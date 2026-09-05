#pragma once

#include "CoreMinimal.h"

/** The project's log category. */
DECLARE_LOG_CATEGORY_EXTERN(LogFathom, Log, All);

/** The trace: `[frame] [world] TAG key=value ...` lines, read by the regression loop. */
DECLARE_LOG_CATEGORY_EXTERN(LogFMTrace, Log, All);
