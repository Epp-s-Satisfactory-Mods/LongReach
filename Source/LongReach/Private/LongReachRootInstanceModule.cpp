#include "LongReachRootInstanceModule.h"
#include "LongReachLogMacros.h"

#include "AbstractInstanceManager.h"
#include "Configuration/ConfigManager.h"
#include "Configuration/ModConfiguration.h"
#include "FGBuildableConveyorBelt.h"
#include "FGBuildGun.h"
#include "FGDropPod.h"
#include "FGInteractActor.h"
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

    SUBSCRIBE_UOBJECT_METHOD(AFGCharacterPlayer, UpdateBestUsableActor, [](auto& scope, AFGCharacterPlayer* self)
        {
            LR_LOG("AFGCharacterPlayer::UpdateBestUsableActor: START");
            auto worldModule = ULongReachRootInstanceModule::GetGameWorldModule();
            float interactDistanceInCM;
            float pickupDistanceInCM;
            worldModule->GetPlayerUseDistances(self, interactDistanceInCM, pickupDistanceInCM);
            float maxUseDistanceInCM = FMath::Max(interactDistanceInCM, pickupDistanceInCM);

            LR_LOG("AFGCharacterPlayer::UpdateBestUsableActor: interactDistanceInCM: %f, pickupDistanceInCM: %f, maxUseDistanceInCM: %f", interactDistanceInCM, pickupDistanceInCM, maxUseDistanceInCM);
            self->mUseDistance = maxUseDistanceInCM;
            scope(self);

            if (!self->mBestUsableActor || FMath::IsNearlyEqual(interactDistanceInCM, pickupDistanceInCM, 50.0))
            {
                LR_LOG("AFGCharacterPlayer::UpdateBestUsableActor: No best usable actor or distances are roughly the same!");
                return;
            }

            LR_LOG("AFGCharacterPlayer::UpdateBestUsableActor: mBestUsableActor: %s", *self->mBestUsableActor->GetName());

            FVector eyesLocation;
            FRotator eyesRotator;
            self->GetActorEyesViewPoint(eyesLocation, eyesRotator);

            auto cachedUseState = self->GetCachedUseState();
            auto distanceSqFromCharacter = FVector::DistSquared(eyesLocation, cachedUseState->UseLocation);

            auto useInteractDistance =
                // Buildables use the interact distance EXCEPT for conveyor belts, because the only interaction is picking things up
                (self->mBestUsableActor->IsA(AFGBuildable::StaticClass()) && !self->mBestUsableActor->IsA(AFGBuildableConveyorBelt::StaticClass()))
                // For dismantle crates and decoration actors
                || self->mBestUsableActor->IsA(AFGInteractActor::StaticClass())
                // Drop pods are interactable too
                || self->mBestUsableActor->IsA(AFGDropPod::StaticClass());

            if (useInteractDistance)
            {
                self->mUseDistance = interactDistanceInCM;
                auto effectiveUseDistanceSq = FMath::Square(self->GetUseDistance());

                LR_LOG("AFGCharacterPlayer::UpdateBestUsableActor: Is interactable!");
                if (effectiveUseDistanceSq < distanceSqFromCharacter)
                {
                    // Rescan using the closer overrideDistance
                    LR_LOG("AFGCharacterPlayer::UpdateBestUsableActor: effectiveUseDistanceSq < distanceSqFromCharacter. Rescanning with mUseDistance %f", self->mUseDistance);
                    scope(self);
                }
            }
            else
            {
                LR_LOG("AFGCharacterPlayer::UpdateBestUsableActor: Is pickupable!");
                self->mUseDistance = pickupDistanceInCM;
                auto effectivePickupDistanceSq = FMath::Square(self->GetUseDistance());
                if (effectivePickupDistanceSq < distanceSqFromCharacter)
                {
                    LR_LOG("AFGCharacterPlayer::UpdateBestUsableActor: effectivePickupDistanceSq < distanceSqFromCharacter. Rescanning with mUseDistance %f", self->mUseDistance);
                    scope(self);
                }
            }
            LR_LOG("AFGCharacterPlayer::UpdateBestUsableActor: mBestUsableActor NOW: %p", self->mBestUsableActor);
            LR_LOG("AFGCharacterPlayer::UpdateBestUsableActor END");
        });

    SUBSCRIBE_UOBJECT_METHOD(AFGBuildGun, TraceForBuilding, [&](auto& scope, const AFGBuildGun* self, APawn* owningPawn, FHitResult& hitresult)
        {
            if (auto owningCharacter = Cast<AFGCharacterPlayer>(owningPawn))
            {
                auto worldModule = ULongReachRootInstanceModule::GetGameWorldModule();
                auto selfMutable = const_cast<AFGBuildGun*>(self);
                selfMutable->mBuildDistanceMax = worldModule->GetPlayerConstructionDistanceInCM(owningCharacter);
            }
            scope(self, owningPawn, hitresult);
        });

    SUBSCRIBE_UOBJECT_METHOD(UFGBuildGunState, GetBuildGunRangeOverride, [&](auto& scope, UFGBuildGunState* self)
        {
            auto overrideDistance = scope(self);
            if (overrideDistance > -1 && self->GetBuildGun()->mPlayerCharacter)
            {
                LR_LOG("UFGBuildGunState::GetBuildGunRangeOverride: Unmodded overrideDistance: %f", overrideDistance);
                auto worldModule = ULongReachRootInstanceModule::GetGameWorldModule();
                auto modConfiguredDistance = worldModule->GetPlayerConstructionDistanceInCM(self->GetBuildGun()->mPlayerCharacter);
                overrideDistance = FMath::Max(overrideDistance, modConfiguredDistance);
                LR_LOG("UFGBuildGunState::GetBuildGunRangeOverride: Setting to overrideDistance: %f", overrideDistance);
                scope.Override(overrideDistance);
            }
            return overrideDistance;
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
                }
                else
                {
                    LR_LOG("UFGBuildGunState::BuildSamplePressed_Implementation: No Instigator character");
                }

                scope(self);
            });
    }
}
