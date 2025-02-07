#include "LongReachUpdateConfigRCO.h"

#include "FGCharacterPlayer.h"
#include "FGPlayerController.h"
#include "LongReachDebugging.h"
#include "LongReachDebugSettings.h"
#include "LongReachLogMacros.h"
#include "LongReachRootInstanceModule.h"
#include "LongReachRootWorldModule.h"
#include "Net/UnrealNetwork.h"

void ULongReachUpdateConfigRCO::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ULongReachUpdateConfigRCO, bDummyToForceReplication);
}

void ULongReachUpdateConfigRCO::SetConfig_Server_Implementation(AFGCharacterPlayer* player, FLongReachConfigurationStruct config)
{
#if LR_DEBUGGING_ENABLED
    LongReachDebugging::DumpPlayer("ULongReachUpdateConfigRCO::SetConfig_Server_Implementation", player);
    LongReachDebugging::DumpConfigStruct("ULongReachUpdateConfigRCO::SetConfig_Server_Implementation", config);
#endif

    ULongReachRootInstanceModule::GetGameWorldModule()->SetConfig(player, config);
}

