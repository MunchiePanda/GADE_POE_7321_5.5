#pragma once

#include "CoreMinimal.h"
#include "PickupBase.h"
#include "SpeedBoostPickup.generated.h"

UCLASS()
class GADE_POE_API ASpeedBoostPickup : public APickupBase
{
    GENERATED_BODY()

protected:
    virtual void ApplyEffect(AActor* Racer) override;

private:
    UPROPERTY(EditAnywhere, Category = "Pickup")
    float SpeedMultiplier = 1.5f;

    float OriginalSpeed; // Store the original speed

    FTimerHandle ResetTimerHandle; // Handle for the timer

    UPROPERTY()
    class AAIRacer* LastAIRacer; // Track the last affected AI racer

    UFUNCTION()
    void ResetPlayerSpeed();

    UFUNCTION()
    void ResetAISpeed();
};