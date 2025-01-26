#pragma once

#include "CoreMinimal.h"

#include "LongReachDebugSettings.h"
#include "LongReachLogCategory.h"

#if LR_DEBUGGING_ENABLED
#define LR_LOG(Format, ...)\
    UE_LOG(LogLongReach, Verbose, TEXT(Format), ##__VA_ARGS__)
#else
#define LR_LOG(Format, ...)
#endif
