#pragma once

#include "LongReachDebugSettings.h"
#include "LongReachDebugging.h"
#include "LongReachLogMacros.h"

#if LR_DEBUGGING_ENABLED
#define LR_DUMP_PLAYER( PREFIX, PLAYER ) \
    LongReachDebugging::DumpPlayer( PREFIX, PLAYER );
#else
#define LR_DUMP_PLAYER( PREFIX, PLAYER )
#endif

#if LR_DEBUGGING_ENABLED
#define LR_DUMP_CONFIG_STRUCT( PREFIX, CONFIG ) \
    LongReachDebugging::DumpConfigStruct( PREFIX, CONFIG );
#else
#define LR_DUMP_PLAYER( PREFIX, PLAYER )
#endif

#if LR_DEBUGGING_ENABLED
#define LR_DUMP_PLAYER_AND_CONFIG_MAP( PREFIX, REASON, PLAYER, MAP ) \
    LR_LOG("%s: %s", *FString(PREFIX), *FString(REASON));\
    LR_DUMP_PLAYER( PREFIX, PLAYER ) \
    LongReachDebugging::DumpConfigMap( PREFIX, MAP);
#else
#define LR_DUMP_PLAYER_CONFIG_NOT_FOUND( PREFIX, REASON, PLAYER, MAP )
#endif
