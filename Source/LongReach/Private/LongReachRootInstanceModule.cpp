#include "LongReachRootInstanceModule.h"
#include "LongReachDebuggingMacros.h"
#include "LongReachLogMacros.h"

#include "AbstractInstanceManager.h"
#include "Configuration/ConfigManager.h"
#include "Configuration/ModConfiguration.h"
#include "FGBuildableConveyorBase.h"
#include "FGBuildGun.h"
#include "FGDriveablePawn.h"
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
        SUBSCRIBE_UOBJECT_METHOD(AFGPlayerController, BeginPlay, [](auto& scope, AFGPlayerController* self)
            {
                LR_LOG("AFGPlayerController::BeginPlay START %s", *self->GetName());
                scope(self);
                auto worldModule = ULongReachRootInstanceModule::GetGameWorldModule();
                worldModule->UpdateConfig();
                LR_LOG("AFGPlayerController::BeginPlay END");
            });
    }

    SUBSCRIBE_UOBJECT_METHOD(AFGCharacterPlayer, UpdateBestUsableActor, [](auto& scope, AFGCharacterPlayer* self)
        {
            LR_LOG("AFGCharacterPlayer::UpdateBestUsableActor: START");
            LR_DUMP_PLAYER(TEXT("AFGCharacterPlayer::UpdateBestUsableActor:"), self);

            auto playerController = self->GetFGPlayerController();
            if (!playerController)
            {
                LR_LOG("AFGCharacterPlayer::UpdateBestUsableActor: AFGCharacterPlayer has no player controller");
                scope(self);
                LR_LOG("AFGCharacterPlayer::UpdateBestUsableActor: END");
                return;
            }

            auto worldModule = ULongReachRootInstanceModule::GetGameWorldModule();
            float interactDistanceInCM;
            float pickupDistanceInCM;
            float vehicleInteractDistanceInCM;
            worldModule->GetPlayerUseDistances(playerController, interactDistanceInCM, pickupDistanceInCM, vehicleInteractDistanceInCM);
            float minUseDistanceInCM = FMath::Min3(interactDistanceInCM, pickupDistanceInCM, vehicleInteractDistanceInCM);
            float maxUseDistanceInCM = FMath::Max3(interactDistanceInCM, pickupDistanceInCM, vehicleInteractDistanceInCM);

            LR_LOG("AFGCharacterPlayer::UpdateBestUsableActor: interactDistanceInCM: %f, pickupDistanceInCM: %f, vehicleInteractDistanceInCM: %f, minUseDistanceInCM: %f, maxUseDistanceInCM: %f",
                interactDistanceInCM,
                pickupDistanceInCM,
                vehicleInteractDistanceInCM,
                minUseDistanceInCM,
                maxUseDistanceInCM);

            self->mUseDistance = maxUseDistanceInCM;
            scope(self);

            if (!self->mBestUsableActor || FMath::IsNearlyEqual(minUseDistanceInCM, maxUseDistanceInCM, 50.0))
            {
                LR_LOG("AFGCharacterPlayer::UpdateBestUsableActor: No best usable actor or distances are roughly the same!");
                LR_LOG("AFGCharacterPlayer::UpdateBestUsableActor: END");
                return;
            }

            LR_LOG("AFGCharacterPlayer::UpdateBestUsableActor: mBestUsableActor: %s", *self->mBestUsableActor->GetName());

            FVector eyesLocation;
            FRotator eyesRotator;
            self->GetActorEyesViewPoint(eyesLocation, eyesRotator);

            auto cachedUseState = self->GetCachedUseState();
            auto distanceFromCharacterSq = FVector::DistSquared(eyesLocation, cachedUseState->UseLocation);
            LR_LOG("AFGCharacterPlayer::UpdateBestUsableActor: Actor is %f units away!", FMath::Sqrt(distanceFromCharacterSq));

            if (UsesInteractDistance(self->mBestUsableActor))
            {
                LR_LOG("AFGCharacterPlayer::UpdateBestUsableActor: Is interactable!");
                if (interactDistanceInCM < maxUseDistanceInCM)
                {
                    LR_LOG("AFGCharacterPlayer::UpdateBestUsableActor:\tInteract distance is not the max distance. Checking whether actor is too far!");
                    self->mUseDistance = interactDistanceInCM;
                    auto effectiveUseDistanceSq = FMath::Square(self->GetUseDistance());
                    LR_LOG("AFGCharacterPlayer::UpdateBestUsableActor:\tEffective use distance is now %f!", self->GetUseDistance());

                    if (effectiveUseDistanceSq < distanceFromCharacterSq) // The actor is outside of the configured distance for its type
                    {
                        LR_LOG("AFGCharacterPlayer::UpdateBestUsableActor:\teffectiveUseDistanceSq < distanceFromCharacterSq. Rescanning with mUseDistance %f", self->mUseDistance);
                        scope(self);
                    }
                }
            }
            else if(UsesVehicleInteractDistance(self->mBestUsableActor))
            {
                LR_LOG("AFGCharacterPlayer::UpdateBestUsableActor: Is vehicle!");
                if (vehicleInteractDistanceInCM < maxUseDistanceInCM)
                {
                    LR_LOG("AFGCharacterPlayer::UpdateBestUsableActor:\tVehicle interact distance is not the max distance. Checking whether actor is too far!");
                    self->mUseDistance = vehicleInteractDistanceInCM;
                    auto effectiveUseDistanceSq = FMath::Square(self->GetUseDistance());
                    LR_LOG("AFGCharacterPlayer::UpdateBestUsableActor:\tEffective use distance is now %f!", self->GetUseDistance());

                    if (effectiveUseDistanceSq < distanceFromCharacterSq) // The actor is outside of the configured distance for its type
                    {
                        LR_LOG("AFGCharacterPlayer::UpdateBestUsableActor:\teffectiveUseDistanceSq < distanceFromCharacterSq. Rescanning with mUseDistance %f", self->mUseDistance);
                        scope(self);
                    }
                }
            }
            else if(pickupDistanceInCM < maxUseDistanceInCM)
            {
                LR_LOG("AFGCharacterPlayer::UpdateBestUsableActor: Is pickupable and pickup distance is not the max distance. Checking whether actor is too far!");
                self->mUseDistance = pickupDistanceInCM;
                auto effectiveUseDistanceSq = FMath::Square(self->GetUseDistance());
                LR_LOG("AFGCharacterPlayer::UpdateBestUsableActor:\tEffective use distance is now %f!", self->GetUseDistance());

                if (effectiveUseDistanceSq < distanceFromCharacterSq) // The actor is outside of the configured distance for its type
                {
                    LR_LOG("AFGCharacterPlayer::UpdateBestUsableActor:\teffectiveUseDistanceSq < distanceFromCharacterSq. Rescanning with mUseDistance %f", self->mUseDistance);
                    scope(self);
                }
            }

            if (self->mBestUsableActor)
            {
                LR_LOG("AFGCharacterPlayer::UpdateBestUsableActor: Final mBestUsableActor: %s", *self->mBestUsableActor->GetName());
            }
            else
            {
                LR_LOG("AFGCharacterPlayer::UpdateBestUsableActor: NO mBestUsableActor");
            }

            LR_LOG("AFGCharacterPlayer::UpdateBestUsableActor: END");
        });

    SUBSCRIBE_UOBJECT_METHOD(AFGBuildGun, TraceForBuilding, [&](auto& scope, const AFGBuildGun* self, APawn* owningPawn, FHitResult& hitresult)
        {
            if (auto owningCharacter = Cast<AFGCharacterPlayer>(owningPawn))
            {
                if (auto playerController = owningCharacter->GetFGPlayerController())
                {
                    auto worldModule = ULongReachRootInstanceModule::GetGameWorldModule();
                    auto selfMutable = const_cast<AFGBuildGun*>(self);
                    selfMutable->mBuildDistanceMax = worldModule->GetPlayerConstructionDistanceInCM(playerController);
                }
            }
            scope(self, owningPawn, hitresult);
        });

    SUBSCRIBE_UOBJECT_METHOD(UFGBuildGunState, GetBuildGunRangeOverride, [&](auto& scope, UFGBuildGunState* self)
        {
            auto overrideDistance = scope(self);
            if (overrideDistance > -1 && self->GetBuildGun()->mPlayerCharacter)
            {
                if (auto playerController = self->GetBuildGun()->mPlayerCharacter->GetFGPlayerController())
                {
                    LR_LOG("UFGBuildGunState::GetBuildGunRangeOverride: Unmodded overrideDistance: %f", overrideDistance);
                    auto worldModule = ULongReachRootInstanceModule::GetGameWorldModule();
                    auto modConfiguredDistance = worldModule->GetPlayerConstructionDistanceInCM(playerController);
                    overrideDistance = FMath::Max(overrideDistance, modConfiguredDistance);
                    LR_LOG("UFGBuildGunState::GetBuildGunRangeOverride: Setting to overrideDistance: %f", overrideDistance);
                    scope.Override(overrideDistance);
                }
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

bool ULongReachRootInstanceModule::UsesInteractDistance(AActor* actor)
{
    return
        // Buildables use the interact distance EXCEPT for conveyor belts/lifts because the only interaction is picking things up
        (actor->IsA(AFGBuildable::StaticClass()) && !actor->IsA(AFGBuildableConveyorBase::StaticClass()))
        // For dismantle crates and decoration actors
        || actor->IsA(AFGInteractActor::StaticClass());
}

bool ULongReachRootInstanceModule::UsesVehicleInteractDistance(AActor* actor)
{
    return actor->IsA(AFGDriveablePawn::StaticClass());
}
