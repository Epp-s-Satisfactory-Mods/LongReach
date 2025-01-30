#include "LongReachUpdateConfigRCO.h"

#include "FGCharacterPlayer.h"
#include "FGPlayerController.h"
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
    LR_LOG("ULongReachUpdateConfigRCO::SetConfig_Server_Implementation: Setting config for player %s (%d). UseDistanceInMeters: %f, BuildOrSampleDistanceInMeters: %f",
        *player->GetPlayerState()->GetPlayerName(),
        player->GetPlayerState()->GetPlayerId(),
        config.UseDistanceInMeters,
        config.BuildOrSampleDistanceInMeters);

    ULongReachRootInstanceModule::GetGameWorldModule()->SetConfig(player, config);
}

