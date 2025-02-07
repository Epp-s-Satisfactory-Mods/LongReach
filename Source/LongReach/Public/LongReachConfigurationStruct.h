#pragma once
#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "LongReachConfigurationStruct.generated.h"

/* Struct generated from Mod Configuration Asset '/LongReach/LongReachConfiguration' */
USTRUCT(BlueprintType)
struct FLongReachConfigurationStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    float InteractDistanceInMeters{};

    UPROPERTY(BlueprintReadWrite)
    float PickupDistanceInMeters{};

    UPROPERTY(BlueprintReadWrite)
    float ConstructionDistanceInMeters{};

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FLongReachConfigurationStruct GetActiveConfig(UObject* WorldContext) {
        FLongReachConfigurationStruct ConfigStruct{};
        FConfigId ConfigId{"LongReach", ""};
        if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull)) {
            UConfigManager* ConfigManager = World->GetGameInstance()->GetSubsystem<UConfigManager>();
            ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FLongReachConfigurationStruct::StaticStruct(), &ConfigStruct});
        }
        return ConfigStruct;
    }
};

