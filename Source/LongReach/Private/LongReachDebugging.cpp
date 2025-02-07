#include "LongReachDebugging.h"

#include "FGCharacterPlayer.h"
#include "FGPlayerState.h"
#include "LongReachLogMacros.h"

void LongReachDebugging::DumpConfigMap(FString prefix, TMap<AFGCharacterPlayer*, FLongReachConfigInCM>& configByPlayer)
{
    EnsureColon(prefix);

    LR_LOG("%s Config map has %d items", *prefix, configByPlayer.Num());
    auto nestedPrefix = GetNestedPrefix(prefix);
    for (auto& kvp : configByPlayer)
    {
        auto player = kvp.Key;
        auto& config = kvp.Value;

        DumpPlayer(nestedPrefix, player);
        DumpConfigInCM(nestedPrefix, config);
    }
}

void LongReachDebugging::DumpPlayer(FString prefix, const AFGCharacterPlayer* player)
{
    EnsureColon(prefix);

    if (!IsValid(player))
    {
        LR_LOG("%s Player is not valid. Pointer: %p", *prefix, player);
        return;
    }

    auto playerState = player->GetPlayerState();

    if (!IsValid(playerState))
    {
        LR_LOG("%s Player %s does not have a valid player state.", *prefix, *player->GetName());
        return;
    }

    LR_LOG("%s Player: %s (%d)",
        *prefix,
        *playerState->GetPlayerName(),
        playerState->GetPlayerId())
}

void LongReachDebugging::DumpConfigInCM(FString prefix, FLongReachConfigInCM& config)
{
    EnsureColon(prefix);

    LR_LOG("%s InteractDistanceInCM: %f, PickupDistanceInCM: %f, ConstructionDistanceInCM: %f",
        *prefix,
        config.InteractDistanceInCM,
        config.PickupDistanceInCM,
        config.ConstructionDistanceInCM);
}

void LongReachDebugging::DumpConfigStruct(FString prefix, FLongReachConfigurationStruct& config)
{
    EnsureColon(prefix);

    LR_LOG("%s InteractDistanceInMeters: %f, PickupDistanceInMeters: %f, ConstructionDistanceInMeters: %f",
        *prefix,
        config.InteractDistanceInMeters,
        config.PickupDistanceInMeters,
        config.ConstructionDistanceInMeters);
}
