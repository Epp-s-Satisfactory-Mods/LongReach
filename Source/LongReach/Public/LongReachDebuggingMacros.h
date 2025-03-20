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
#define LR_DUMP_PLAYER_CONTROLLER( PREFIX, PLAYER_CONTROLLER ) \
    LongReachDebugging::DumpPlayerController( PREFIX, PLAYER_CONTROLLER );
#else
#define LR_DUMP_PLAYER_CONTROLLER( PREFIX, PLAYER_CONTROLLER )
#endif

#if LR_DEBUGGING_ENABLED
#define LR_DUMP_CONFIG_STRUCT( PREFIX, CONFIG ) \
    LongReachDebugging::DumpConfigStruct( PREFIX, CONFIG );
#else
#define LR_DUMP_CONFIG_STRUCT( PREFIX, CONFIG )
#endif

#if LR_DEBUGGING_ENABLED
#define LR_DUMP_PLAYER_CONTROLLER_AND_CONFIG_MAP( PREFIX, REASON, PLAYER_CONTROLLER, MAP ) \
    LR_LOG("%s: %s", *FString(PREFIX), *FString(REASON));\
    LR_DUMP_PLAYER_CONTROLLER(PREFIX, PLAYER_CONTROLLER) \
    LongReachDebugging::DumpConfigMap( PREFIX, MAP);
#else
#define LR_DUMP_PLAYER_CONTROLLER_AND_CONFIG_MAP( PREFIX, REASON, PLAYER_CONTROLLER, MAP )
#endif
