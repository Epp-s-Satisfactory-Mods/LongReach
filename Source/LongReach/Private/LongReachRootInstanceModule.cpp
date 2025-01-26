#include "LongReachRootInstanceModule.h"
#include "LongReachLogMacros.h"

#include "FGBuildGun.h"
#include "FGCharacterPlayer.h"
#include "Kismet/KismetMathLibrary.h"
#include "Patching/NativeHookManager.h"

void ULongReachRootInstanceModule::DispatchLifecycleEvent(ELifecyclePhase phase)
{
    if (phase != ELifecyclePhase::INITIALIZATION)
    {
        Super::DispatchLifecycleEvent(phase);
        return;
    }

    LR_LOG("ULongReachRootInstanceModule::DispatchLifecycleEvent: Registering mod hooks...");

    SUBSCRIBE_UOBJECT_METHOD(AFGCharacterPlayer, GetUseDistance, [](auto& scope, const AFGCharacterPlayer* self)
        {
            LR_LOG("AFGCharacterPlayer::GetUseDistance START %s", *self->GetName());

            auto useDistance = scope(self);
            LR_LOG("AFGCharacterPlayer::GetUseDistance: Normal distance: %f", useDistance);
            useDistance = FMath::Max(useDistance, 2000);
            LR_LOG("AFGCharacterPlayer::GetUseDistance END: Overriding with distance: %f", useDistance);
            scope.Override(useDistance);
            return useDistance;
        });


    SUBSCRIBE_UOBJECT_METHOD(AFGBuildGun, BeginPlay, [&](auto& scope, AFGBuildGun* self)
        {
            LR_LOG("AFGBuildGun::BeginPlay START %s", *self->GetName());
            self->mBuildDistanceMax = 100000;
            scope(self);
            LR_LOG("AFGBuildGun::BeginPlay END");
        });

    Super::DispatchLifecycleEvent(phase);
}
