#pragma once

#include "CoreMinimal.h"
#include "LongReachConfigurationStruct.h"
#include "LongReachUpdateConfigRCO.h"
#include "LongReachLogMacros.h"
#include "Module/GameWorldModule.h"
#include "LongReachRootWorldModule.generated.h"

struct FLongReachConfigInCM {
public:
    float UseDistanceInCM;
    float BuildOrSampleDistanceInCM;
};

UCLASS()
class LONGREACH_API ULongReachRootWorldModule : public UGameWorldModule
{
    GENERATED_BODY()

public:
    float GetPlayerUseDistanceInCM(const AFGCharacterPlayer* player);
    float GetPlayerBuildOrSampleDistanceInCM(const AFGCharacterPlayer* player);
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
