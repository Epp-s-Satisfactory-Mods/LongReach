#pragma once

#include "CoreMinimal.h"
#include "LongReachConfigInCM.h"
#include "LongReachConfigurationStruct.h"
#include "LongReachUpdateConfigRCO.h"
#include "LongReachLogMacros.h"
#include "ModConfiguration.h"
#include "Module/GameWorldModule.h"
#include "LongReachRootWorldModule.generated.h"

UCLASS()
class LONGREACH_API ULongReachRootWorldModule : public UGameWorldModule
{
    GENERATED_BODY()

public:
    void GetPlayerUseDistances(
        const AFGCharacterPlayer* player,
        float& useDistanceInCM,
        float& pickupDistanceInCM);

    float GetPlayerConstructionDistanceInCM(const AFGCharacterPlayer* player);

    inline static FConfigId ConfigId{ "LongReach", "" };

    void SetConfig(AFGCharacterPlayer* player, FLongReachConfigurationStruct config);

    UFUNCTION() // This must be a UFUNCTION so it can subscribe to config update broadcasts
    void UpdateConfig();

protected:
    virtual void DispatchLifecycleEvent(ELifecyclePhase phase) override;

    UPROPERTY()
    ULongReachUpdateConfigRCO* UpdateConfigRCO;

    UPROPERTY()
    AFGCharacterPlayer* Player;

    TMap<AFGCharacterPlayer*, FLongReachConfigInCM> ConfigByPlayer;
};
