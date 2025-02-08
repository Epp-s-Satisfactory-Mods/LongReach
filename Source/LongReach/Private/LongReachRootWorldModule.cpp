#include "LongReachRootWorldModule.h"

#include "Configuration/ConfigManager.h"
#include "Configuration/ModConfiguration.h"
#include "FGCharacterPlayer.h"
#include "FGPlayerController.h"
#include "LongReachDebugging.h"
#include "LongReachDebugSettings.h"
#include "LongReachLogMacros.h"
#include "LongReachRootInstanceModule.h"

#if LR_DEBUGGING_ENABLED
#define LR_DUMP_PLAYER_CONFIG_NOT_FOUND( PREFIX, PLAYER, MAP ) \
    LR_LOG("%s: No config found for player!", *FString(PREFIX));\
    LongReachDebugging::DumpPlayer( PREFIX, PLAYER ); \
    LongReachDebugging::DumpConfigMap( PREFIX, MAP);
#else
#define LR_DUMP_PLAYER_CONFIG_NOT_FOUND( PREFIX, PLAYER, MAP )
#endif

void ULongReachRootWorldModule::GetPlayerUseDistances(
    const AFGCharacterPlayer* player,
    float& interactDistanceInCM,
    float& pickupDistanceInCM)
{
    auto config = this->ConfigByPlayer.Find(player);

    if (!config)
    {
        LR_DUMP_PLAYER_CONFIG_NOT_FOUND(TEXT("ULongReachRootWorldModule::GetPlayerUseDistances"), player, this->ConfigByPlayer);

        // Return unmodded default
        interactDistanceInCM = pickupDistanceInCM = 450.0f;
        return;
    }

    interactDistanceInCM = config->InteractDistanceInCM;
    pickupDistanceInCM = config->PickupDistanceInCM;
}

float ULongReachRootWorldModule::GetPlayerConstructionDistanceInCM(const AFGCharacterPlayer* player)
{
    LR_LOG("ULongReachRootWorldModule::GetPlGetPlayerConstructionDistanceInCMayerUseDistances. Player: %p", player);
    LR_LOG("ULongReachRootWorldModule::GetPlayerConstructionDistanceInCM. Player is valid: %d", IsValid(player));
    auto config = this->ConfigByPlayer.Find(player);

    LR_LOG("ULongReachRootWorldModule::GetPlayerConstructionDistanceInCM. Config: %p", config);
    if (!config)
    {
        LR_DUMP_PLAYER_CONFIG_NOT_FOUND(TEXT("ULongReachRootWorldModule::GetPlayerConstructionDistanceInCM"), player, this->ConfigByPlayer);

        // Return unmodded default
        return 1000.0;
    }

    //LR_LOG("ULongReachRootWorldModule::GetPlayerConstructionDistanceInCM: Returning config value: %f.", config->InteractDistanceInCM);

    return config->ConstructionDistanceInCM;
}

void ULongReachRootWorldModule::SetConfig(AFGCharacterPlayer* player, FLongReachConfigurationStruct config)
{
    LR_LOG("ULongReachRootWorldModule::SetConfig: Setting config for player %s (%d). InteractDistanceInMeters: %f, PickupDistanceInMeters: %f, ConstructionDistanceInMeters: %f",
        *player->GetPlayerState()->GetPlayerName(),
        player->GetPlayerState()->GetPlayerId(),
        config.InteractDistanceInMeters,
        config.PickupDistanceInMeters,
        config.ConstructionDistanceInMeters);

    auto configInCM = FLongReachConfigInCM();
    configInCM.InteractDistanceInCM = config.InteractDistanceInMeters * 100;
    configInCM.PickupDistanceInCM = config.PickupDistanceInMeters * 100;
    configInCM.ConstructionDistanceInCM = config.ConstructionDistanceInMeters * 100;

    this->ConfigByPlayer.Emplace(player, configInCM);
}

void ULongReachRootWorldModule::UpdateConfig()
{
    // Some things are done on the client and some on the server. The most reliable thing is to
    // have the client update locally and also send update to the server.

    // Note that we do some initialization the first time this funtion is called because we can't
    // use AFGPlayerController to initialize things in DispatchLifecycleEvent - it's not available
    // that early when connecting to a remote server.

    LR_LOG("ULongReachRootWorldModule::UpdateConfig: Retrieving active mod config...");
    auto config = FLongReachConfigurationStruct::GetActiveConfig(this->GetWorld());

    if (!IsValid(this->Player))
    {
        LR_LOG("ULongReachRootWorldModule::UpdateConfig: Initializing Player reference...");
        auto firstPlayerController = Cast<AFGPlayerController>(this->GetWorld()->GetFirstPlayerController());
        this->Player = Cast<AFGCharacterPlayer>(firstPlayerController->GetControlledCharacter());
    }

    LR_LOG("ULongReachRootWorldModule::UpdateConfig: Retrieved config. Setting locally.");
    this->SetConfig(this->Player, config);

    if (this->GetWorld()->GetNetMode() < ENetMode::NM_Client)
    {
        LR_LOG("ULongReachRootWorldModule::UpdateConfig: We're the server so we don't need to send config anywhere.");
        return;
    }

    if (!IsValid(this->UpdateConfigRCO))
    {
        LR_LOG("ULongReachRootWorldModule::UpdateConfig: Initializing RCO...");
        auto firstPlayerController = Cast<AFGPlayerController>(this->GetWorld()->GetFirstPlayerController());
        this->UpdateConfigRCO = firstPlayerController->GetRemoteCallObjectOfClass<ULongReachUpdateConfigRCO>();
    }

    LR_LOG("ULongReachRootWorldModule::UpdateConfig: Sending config to server.");
    this->UpdateConfigRCO->SetConfig_Server(this->Player, config);
}

void ULongReachRootWorldModule::DispatchLifecycleEvent(ELifecyclePhase phase)
{
    switch (phase)
    {
        case ELifecyclePhase::INITIALIZATION:
            ULongReachRootInstanceModule::SetGameWorldModule(this);

            LR_LOG("ULongReachRootWorldModule::DispatchLifecycleEvent: Subscribing to config updates...");
            auto gameInstance = this->GetWorld()->GetGameInstance();
            auto configManager = gameInstance->GetSubsystem<UConfigManager>();
            auto modConfigRoot = configManager->GetConfigurationRootSection(ConfigId);

            for (auto& sectionProperty : modConfigRoot->SectionProperties)
            {
                auto& propertyName = sectionProperty.Key;
                auto configProperty = sectionProperty.Value;

                LR_LOG("ULongReachRootWorldModule::DispatchLifecycleEvent: Examining config property %s", *propertyName);

                if (propertyName.Equals(TEXT("InteractDistanceInMeters")) ||
                    propertyName.Equals(TEXT("PickupDistanceInMeters")) ||
                    propertyName.Equals(TEXT("ConstructionDistanceInMeters")) )
                {
                    LR_LOG("ULongReachRootWorldModule::DispatchLifecycleEvent: \tSubscribing to changes on %s, property at %p!", *propertyName, configProperty);
                    configProperty->OnPropertyValueChanged.AddDynamic(this, &ULongReachRootWorldModule::UpdateConfig);
                }
            }
            LR_LOG("ULongReachRootWorldModule::DispatchLifecycleEvent: Done subscribing to config updates.");
            break;
    }

    Super::DispatchLifecycleEvent(phase);
}

#undef LR_DUMP_PLAYER_CONFIG_NOT_FOUND