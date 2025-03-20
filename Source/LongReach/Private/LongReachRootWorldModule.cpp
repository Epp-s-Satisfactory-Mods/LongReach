#include "LongReachRootWorldModule.h"

#include "Configuration/ConfigManager.h"
#include "Configuration/ModConfiguration.h"
//#include "FGCharacterPlayer.h"
#include "FGPlayerController.h"
#include "LongReachDebuggingMacros.h"
#include "LongReachDebugSettings.h"
#include "LongReachLogMacros.h"
#include "LongReachRootInstanceModule.h"

void ULongReachRootWorldModule::GetPlayerUseDistances(
    const AFGPlayerController* playerController,
    float& interactDistanceInCM,
    float& pickupDistanceInCM,
    float& vehicleInteractDistanceInCM)
{
    auto config = this->ConfigByPlayerController.Find(playerController);

    if (!config)
    {
        LR_DUMP_PLAYER_CONTROLLER_AND_CONFIG_MAP(
            TEXT("ULongReachRootWorldModule::GetPlayerUseDistances"),
            TEXT("No config found for player controller!"),
            playerController,
            this->ConfigByPlayerController);

        // Return unmodded default
        interactDistanceInCM = pickupDistanceInCM = 450.0f;
        return;
    }

    interactDistanceInCM = config->InteractDistanceInCM;
    pickupDistanceInCM = config->PickupDistanceInCM;
    vehicleInteractDistanceInCM = config->VehicleInteractDistanceInCM;
}

float ULongReachRootWorldModule::GetPlayerConstructionDistanceInCM(const AFGPlayerController* playerController)
{
    auto config = this->ConfigByPlayerController.Find(playerController);

    LR_LOG("ULongReachRootWorldModule::GetPlayerConstructionDistanceInCM. Config: %p", config);
    if (!config)
    {
        LR_DUMP_PLAYER_CONTROLLER_AND_CONFIG_MAP(
            TEXT("ULongReachRootWorldModule::GetPlayerConstructionDistanceInCM"),
            TEXT("No config found for playerController!"),
            playerController,
            this->ConfigByPlayerController);

        // Return unmodded default
        return 1000.0;
    }

    //LR_LOG("ULongReachRootWorldModule::GetPlayerConstructionDistanceInCM: Returning config value: %f.", config->InteractDistanceInCM);

    return config->ConstructionDistanceInCM;
}

void ULongReachRootWorldModule::SetConfig(AFGPlayerController* playerController, FLongReachConfigurationStruct config)
{
    LR_DUMP_PLAYER_CONTROLLER(TEXT("ULongReachRootWorldModule::SetConfig"), playerController);
    LR_DUMP_CONFIG_STRUCT(TEXT("ULongReachRootWorldModule::SetConfig"), config);

    auto configInCM = FLongReachConfigInCM();
    configInCM.InteractDistanceInCM = config.InteractDistanceInMeters * 100;
    configInCM.PickupDistanceInCM = config.PickupDistanceInMeters * 100;
    configInCM.ConstructionDistanceInCM = config.ConstructionDistanceInMeters * 100;
    configInCM.VehicleInteractDistanceInCM = config.VehicleInteractDistanceInMeters * 100;

    this->ConfigByPlayerController.Emplace(playerController, configInCM);
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

    if (!IsValid(this->PlayerController))
    {
        LR_LOG("ULongReachRootWorldModule::UpdateConfig: Initializing PlayerController reference...");
        this->PlayerController = Cast<AFGPlayerController>(this->GetWorld()->GetFirstPlayerController());
    }

    LR_LOG("ULongReachRootWorldModule::UpdateConfig: Retrieved config. Setting locally.");
    this->SetConfig(this->PlayerController, config);

    if (this->GetWorld()->GetNetMode() < ENetMode::NM_Client)
    {
        LR_LOG("ULongReachRootWorldModule::UpdateConfig: We're the server so we don't need to send config anywhere.");
        return;
    }

    if (!IsValid(this->UpdateConfigRCO))
    {
        LR_LOG("ULongReachRootWorldModule::UpdateConfig: Initializing RCO...");
        this->UpdateConfigRCO = this->PlayerController->GetRemoteCallObjectOfClass<ULongReachUpdateConfigRCO>();
    }

    LR_LOG("ULongReachRootWorldModule::UpdateConfig: Sending config to server.");
    this->UpdateConfigRCO->SetConfig_Server(this->PlayerController, config);
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
                    propertyName.Equals(TEXT("ConstructionDistanceInMeters")) ||
                    propertyName.Equals(TEXT("VehicleInteractDistanceInMeters")) )
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
