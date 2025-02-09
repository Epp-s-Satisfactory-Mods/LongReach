#pragma once

#include "CoreMinimal.h"
#include "FGRemoteCallObject.h"
#include "FGPlayerController.h"
#include "LongReachConfigurationStruct.h"

#include "LongReachUpdateConfigRCO.generated.h"

UCLASS()
class LONGREACH_API ULongReachUpdateConfigRCO : public UFGRemoteCallObject
{
    GENERATED_BODY()

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(Server, Reliable)
    void SetConfig_Server(AFGPlayerController* playerController, FLongReachConfigurationStruct config);

    UPROPERTY(Replicated)
    bool bDummyToForceReplication = true;
};
