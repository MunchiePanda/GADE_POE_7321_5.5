#pragma once

#include "CoreMinimal.h"
#include "PickupBase.h"
#include "PlayerHamster.h"
#include "AIRacer.h"
#include "SpeedReductionPickup.generated.h"

UCLASS()
class GADE_POE_API ASpeedReductionPickup : public APickupBase
{
    GENERATED_BODY()

protected:
    virtual void ApplyEffect(AActor* Racer) override;

private:
    UPROPERTY(EditAnywhere, Category = "Pickup")
    float SpeedMultiplier = 0.5f;

    UPROPERTY()
    APlayerHamster* AffectedPlayer; // Track affected player

    UPROPERTY()
    AAIRacer* AffectedAIRacer; // Track affected AI racer

    float OriginalSpeed; // Store the original speed

    FTimerHandle ResetTimerHandle; // Handle for the timer

    UFUNCTION()
    void ResetSpeed();
};