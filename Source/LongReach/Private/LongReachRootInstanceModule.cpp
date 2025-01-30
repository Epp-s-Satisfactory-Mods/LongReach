#include "LongReachRootInstanceModule.h"
#include "LongReachLogMacros.h"

#include "AbstractInstanceManager.h"
#include "Configuration/ConfigManager.h"
#include "Configuration/ModConfiguration.h"
#include "FGBuildGun.h"
#include "FGCharacterPlayer.h"
#include "FGPlayerState.h"
#include "Kismet/KismetMathLibrary.h"
#include "Patching/NativeHookManager.h"

ULongReachRootWorldModule* ULongReachRootInstanceModule::CurrentGameWorldModule = nullptr;

void ULongReachRootInstanceModule::DispatchLifecycleEvent(ELifecyclePhase phase)
{
    switch (phase)
    {
    case ELifecyclePhase::INITIALIZATION:
        RegisterModHooks();
        break;
    }

    Super::DispatchLifecycleEvent(phase);
}

void ULongReachRootInstanceModule::RegisterModHooks()
{
    LR_LOG("ULongReachRootInstanceModule::RegisterModHooks: Registering mod hooks...");

    if (!IsRunningDedicatedServer())
    {
        SUBSCRIBE_UOBJECT_METHOD(AFGCharacterPlayer, BeginPlay, [](auto& scope, AFGCharacterPlayer* self)
            {
                LR_LOG("AFGCharacterPlayer::BeginPlay START %s", *self->GetName());
                auto worldModule = ULongReachRootInstanceModule::GetGameWorldModule();
                worldModule->UpdateConfig();
                scope(self);
                LR_LOG("AFGCharacterPlayer::BeginPlay END");
            });
    }

    SUBSCRIBE_UOBJECT_METHOD(AFGCharacterPlayer, GetUseDistance, [](auto& scope, const AFGCharacterPlayer* self)
        {
            //LR_LOG("AFGCharacterPlayer::GetUseDistance START %s", *self->GetName());
            auto useDistance = scope(self);
            //LR_LOG("AFGCharacterPlayer::GetUseDistance: Normal distance: %f", useDistance);
            auto worldModule = ULongReachRootInstanceModule::GetGameWorldModule();
            auto configuredUseDistance = worldModule->GetPlayerUseDistanceInCM(self);
            useDistance = FMath::Max(useDistance, configuredUseDistance);
            //LR_LOG("AFGCharacterPlayer::GetUseDistance END: Overriding with distance: %f", useDistance);
            scope.Override(useDistance);
            return useDistance;
        });

    SUBSCRIBE_UOBJECT_METHOD(AFGBuildGun, TraceForBuilding, [&](auto& scope, const AFGBuildGun* self, APawn* owningPawn, FHitResult& hitresult)
        {
            if (auto owningCharacter = Cast<AFGCharacterPlayer>(owningPawn))
            {
                auto worldModule = ULongReachRootInstanceModule::GetGameWorldModule();
                auto selfMutable = const_cast<AFGBuildGun*>(self);
                selfMutable->mBuildDistanceMax = worldModule->GetPlayerBuildOrSampleDistanceInCM(owningCharacter);
            }
            scope(self, owningPawn, hitresult);
        });

    SUBSCRIBE_UOBJECT_METHOD(UFGBuildGunState, GetBuildGunRangeOverride, [&](auto& scope, UFGBuildGunState* self)
        {
            auto distance = scope(self);
            if (distance > -1 && self->GetBuildGun()->mPlayerCharacter)
            {
                LR_LOG("UFGBuildGunState::GetBuildGunRangeOverride: Unmodded distance: %f", distance);
                auto worldModule = ULongReachRootInstanceModule::GetGameWorldModule();
                auto configuredDistance = worldModule->GetPlayerBuildOrSampleDistanceInCM(self->GetBuildGun()->mPlayerCharacter);
                distance = FMath::Max(distance, configuredDistance);
                LR_LOG("UFGBuildGunState::GetBuildGunRangeOverride: Setting to distance: %f", distance);
                scope.Override(distance);
            }
            return distance;
        });

    if (LR_DEBUGGING_ENABLED)
    {
        SUBSCRIBE_UOBJECT_METHOD(
            UFGBuildGunState,
            BuildSamplePressed_Implementation,
            [](auto& scope, UFGBuildGunState* self)
            {
                auto instigatorCharactor = self->GetBuildGun()->GetInstigatorCharacter();
                if (instigatorCharactor)
                {
                    auto& hitResult = self->GetBuildGun()->GetHitResult();
                    auto playerLocation = instigatorCharactor->GetRealActorLocation();
                    auto distance = FVector::Distance(playerLocation, hitResult.Location);
                    LR_LOG("UFGBuildGunState::BuildSamplePressed_Implementation. Hit result distance from player: %f", distance);

                    auto worldModule = ULongReachRootInstanceModule::GetGameWorldModule();
                    LR_LOG("UFGBuildGunState::BuildSamplePressed_Implementation: Current config: UseDistanceInCM: %f, BuildOrSampleDistanceInCM: %f",
                        worldModule->GetPlayerUseDistanceInCM(instigatorCharactor),
                        worldModule->GetPlayerBuildOrSampleDistanceInCM(instigatorCharactor));
                }
                else
                {
                    LR_LOG("UFGBuildGunState::BuildSamplePressed_Implementation: No Instigator character");
                }

                scope(self);
            });
    }
}
