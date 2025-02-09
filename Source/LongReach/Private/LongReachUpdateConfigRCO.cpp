#include "LongReachUpdateConfigRCO.h"

#include "FGCharacterPlayer.h"
#include "FGPlayerController.h"
#include "LongReachDebuggingMacros.h"
#include "LongReachLogMacros.h"
#include "LongReachRootInstanceModule.h"
#include "LongReachRootWorldModule.h"
#include "Net/UnrealNetwork.h"

void ULongReachUpdateConfigRCO::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ULongReachUpdateConfigRCO, bDummyToForceReplication);
}

void ULongReachUpdateConfigRCO::SetConfig_Server_Implementation(AFGPlayerController* playerController, FLongReachConfigurationStruct config)
{
    LR_DUMP_PLAYER_CONTROLLER(TEXT("ULongReachUpdateConfigRCO::SetConfig_Server_Implementation"), playerController);
    LR_DUMP_CONFIG_STRUCT(TEXT("ULongReachUpdateConfigRCO::SetConfig_Server_Implementation"), config);

    ULongReachRootInstanceModule::GetGameWorldModule()->SetConfig(playerController, config);
}

