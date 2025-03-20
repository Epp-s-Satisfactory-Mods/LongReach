#pragma once

#include "CoreMinimal.h"
#include "LongReachRootWorldModule.h"
#include "Module/GameInstanceModule.h"

#include "LongReachRootInstanceModule.generated.h"

UCLASS()
class LONGREACH_API ULongReachRootInstanceModule : public UGameInstanceModule
{
    GENERATED_BODY()

public:
    void DispatchLifecycleEvent(ELifecyclePhase phase) override;
    static void RegisterModHooks();
    static FORCEINLINE bool UsesInteractDistance(AActor* actor);
    static FORCEINLINE bool UsesVehicleInteractDistance(AActor* actor);

    static ULongReachRootWorldModule* GetGameWorldModule()
    {
        return CurrentGameWorldModule;
    }

    static void SetGameWorldModule(ULongReachRootWorldModule* module)
    {
        CurrentGameWorldModule = module;
    }

protected:
    static ULongReachRootWorldModule* CurrentGameWorldModule;
};
