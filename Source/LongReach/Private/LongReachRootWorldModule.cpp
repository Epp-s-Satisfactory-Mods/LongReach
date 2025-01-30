#include "LongReachRootWorldModule.h"

#include "Configuration/ConfigManager.h"
#include "Configuration/ModConfiguration.h"
#include "FGCharacterPlayer.h"
#include "FGPlayerController.h"
#include "LongReachLogMacros.h"
#include "LongReachRootInstanceModule.h"

float ULongReachRootWorldModule::GetPlayerUseDistanceInCM(const AFGCharacterPlayer* player)
{
    auto config = this->ConfigByPlayer.Find(player);

    if (!config)
    {
        LR_LOG("ULongReachRootWorldModule::GetPlayerBuildOrSampleDistanceInCM: No config found for player %s (%d)",
            *player->GetPlayerState()->GetPlayerName(),
            player->GetPlayerState()->GetPlayerId());

        for (auto& kvp : this->ConfigByPlayer)
        {
            LR_LOG("ULongReachRootWorldModule::GetPlayerBuildOrSampleDistanceInCM: Config for player %s (%d): UseDistanceInMeters: %f, BuildOrSampleDistanceInMeters: %f",
                *kvp.Key->GetPlayerState()->GetPlayerName(),
                kvp.Key->GetPlayerState()->GetPlayerId(),
                kvp.Value.UseDistanceInCM,
                kvp.Value.BuildOrSampleDistanceInCM);
        }

        // Return unmodded default
        return 450.0;
    }

    //LR_LOG("ULongReachRootWorldModule::GetPlayerUseDistanceInCM: Returning config value: %f.", config->UseDistanceInCM);

    return config->UseDistanceInCM;
}

float ULongReachRootWorldModule::GetPlayerBuildOrSampleDistanceInCM(const AFGCharacterPlayer* player)
{
    auto config = this->ConfigByPlayer.Find(player);

    if (!config)
    {
        LR_LOG("ULongReachRootWorldModule::GetPlayerBuildOrSampleDistanceInCM: No config found for player %s (%d)",
            *player->GetPlayerState()->GetPlayerName(),
            player->GetPlayerState()->GetPlayerId());

        for (auto& kvp : this->ConfigByPlayer)
        {
            LR_LOG("ULongReachRootWorldModule::GetPlayerBuildOrSampleDistanceInCM: Config for player %s (%d): UseDistanceInMeters: %f, BuildOrSampleDistanceInMeters: %f",
                *kvp.Key->GetPlayerState()->GetPlayerName(),
                kvp.Key->GetPlayerState()->GetPlayerId(),
                kvp.Value.UseDistanceInCM,
                kvp.Value.BuildOrSampleDistanceInCM);
        }

        // Return unmodded default
        return 10000.0;
    }

    //LR_LOG("ULongReachRootWorldModule::GetPlayerBuildOrSampleDistanceInCM: Returning config value: %f.", config->BuildOrSampleDistanceInCM);

    return config->BuildOrSampleDistanceInCM;
}

void ULongReachRootWorldModule::SetConfig(AFGCharacterPlayer* player, FLongReachConfigurationStruct config)
{
    LR_LOG("ULongReachRootWorldModule::SetConfig: Setting config for player %s (%d). UseDistanceInMeters: %f, BuildOrSampleDistanceInMeters: %f",
        *player->GetPlayerState()->GetPlayerName(),
        player->GetPlayerState()->GetPlayerId(),
        config.UseDistanceInMeters,
        config.BuildOrSampleDistanceInMeters);

    auto configInCM = FLongReachConfigInCM();
    configInCM.UseDistanceInCM = config.UseDistanceInMeters * 100;
    configInCM.BuildOrSampleDistanceInCM = config.BuildOrSampleDistanceInMeters * 100;

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
            auto modConfigRoot = configManager->GetConfigurationRootSection(FLongReachConfigurationStruct::ConfigId);

            for (auto& sectionProperty : modConfigRoot->SectionProperties)
            {
                auto& propertyName = sectionProperty.Key;
                auto configProperty = sectionProperty.Value;

                LR_LOG("ULongReachRootWorldModule::DispatchLifecycleEvent: Examining config property %s", *propertyName);

                if (propertyName.Equals(TEXT("UseDistanceInMeters")) || propertyName.Equals(TEXT("BuildOrSampleDistanceInMeters")))
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
