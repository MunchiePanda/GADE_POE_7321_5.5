#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupBase.generated.h"

UCLASS()
class GADE_POE_API APickupBase : public AActor
{
    GENERATED_BODY()

public:
    APickupBase();

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UPROPERTY(VisibleAnywhere)
    class UBoxComponent* TriggerVolume;

    UPROPERTY(EditAnywhere, Category = "Pickup")
    float EffectDuration = 5.0f;

    UPROPERTY(EditAnywhere, Category = "Pickup")
    class USoundBase* PickupSound;

private:
    bool bIsActive = true;

    UFUNCTION()
    void DeactivatePickup();

    UFUNCTION()
    void ReactivatePickup();

    virtual void ApplyEffect(AActor* Racer) PURE_VIRTUAL(APickupBase::ApplyEffect, );

    FTimerHandle TimerHandle;
};