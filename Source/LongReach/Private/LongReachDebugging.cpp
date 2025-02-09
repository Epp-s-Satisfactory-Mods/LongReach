#include "LongReachDebugging.h"

#include "FGCharacterPlayer.h"
#include "FGPlayerController.h"
#include "FGPlayerState.h"
#include "LongReachLogMacros.h"

void LongReachDebugging::DumpConfigMap(FString prefix, TMap<AFGPlayerController*, FLongReachConfigInCM>& configByPlayer)
{
    EnsureColon(prefix);

    LR_LOG("%s Config map has %d items", *prefix, configByPlayer.Num());
    auto nestedPrefix = GetNestedPrefix(prefix);
    for (auto& kvp : configByPlayer)
    {
        auto playerController = kvp.Key;
        auto& config = kvp.Value;

        DumpPlayerController(nestedPrefix, playerController);
        DumpConfigInCM(nestedPrefix, config);
    }
}

void LongReachDebugging::DumpPlayer(FString prefix, const AFGCharacterPlayer* player)
{
    EnsureColon(prefix);

    if (!IsValid(player))
    {
        LR_LOG("%s AFGCharacterPlayer is not valid. Pointer: %p", *prefix, player);
        return;
    }

    auto playerState = player->GetPlayerState();

    if (!IsValid(playerState))
    {
        LR_LOG("%s AFGCharacterPlayer %s does not have a valid player state.", *prefix, *player->GetName());
        return;
    }

    LR_LOG("%s Player: %s (%d)",
        *prefix,
        *playerState->GetPlayerName(),
        playerState->GetPlayerId())
}

void LongReachDebugging::DumpPlayerController(FString prefix, const AFGPlayerController* playerController)
{
    EnsureColon(prefix);

    if (!IsValid(playerController))
    {
        LR_LOG("%s AFGPlayerController is not valid. Pointer: %p", *prefix, playerController);
        return;
    }

    auto player = Cast<AFGCharacterPlayer>(playerController->GetControlledCharacter());
    auto& nestedPrefix = GetNestedPrefix(prefix).Append("GetControlledCharacter");
    DumpPlayer(nestedPrefix, player);
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
