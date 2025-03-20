#pragma once

#include "CoreMinimal.h"
#include "FGPlayerController.h"
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
        const AFGPlayerController* player,
        float& useDistanceInCM,
        float& pickupDistanceInCM,
        float& vehicleInteractDistanceInCM);

    float GetPlayerConstructionDistanceInCM(const AFGPlayerController* player);

    inline static FConfigId ConfigId{ "LongReach", "" };

    void SetConfig(AFGPlayerController* player, FLongReachConfigurationStruct config);

    UFUNCTION() // This must be a UFUNCTION so it can subscribe to config update broadcasts
    void UpdateConfig();

protected:
    virtual void DispatchLifecycleEvent(ELifecyclePhase phase) override;

    UPROPERTY()
    ULongReachUpdateConfigRCO* UpdateConfigRCO;

    UPROPERTY()
    AFGPlayerController* PlayerController;

    TMap<AFGPlayerController*, FLongReachConfigInCM> ConfigByPlayerController;
};
