#pragma once

#include "CoreMinimal.h"
#include "Module/GameInstanceModule.h"

#include "LongReachRootInstanceModule.generated.h"

UCLASS()
class LONGREACH_API ULongReachRootInstanceModule : public UGameInstanceModule
{
    GENERATED_BODY()
public:
    ULongReachRootInstanceModule()
        : UGameInstanceModule()
    {
        this->OverrideBuildSampleDistanceOnNextCall = false;
    }

    void DispatchLifecycleEvent(ELifecyclePhase phase) override;

    bool OverrideBuildSampleDistanceOnNextCall;
};
