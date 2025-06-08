#include "SpeedReductionPickup.h"
#include "PlayerHamster.h"
#include "AIRacer.h"
#include "AIRacerContoller.h"
#include "Kismet/GameplayStatics.h"

void ASpeedReductionPickup::ApplyEffect(AActor* Racer)
{
    // Reset affected actors and original speed
    AffectedPlayer = nullptr;
    AffectedAIRacer = nullptr;
    OriginalSpeed = 0.0f;

    // Handle player character speed reduction
    if (APlayerHamster* Player = Cast<APlayerHamster>(Racer))
    {
        // Store original speed and apply reduction
        OriginalSpeed = Player->GetSpeed();
        Player->SetSpeed(OriginalSpeed * SpeedMultiplier);
        AffectedPlayer = Player;
        UE_LOG(LogTemp, Log, TEXT("SpeedReductionPickup: Reduced Player speed from %f to %f"), OriginalSpeed, Player->GetSpeed());
    }
    // Handle AI character speed reduction
    else if (AAIRacer* AIRacer = Cast<AAIRacer>(Racer))
    {
        // Store original speed and apply reduction
        OriginalSpeed = AIRacer->MaxSpeed;
        AIRacer->MaxSpeed *= SpeedMultiplier;
        AffectedAIRacer = AIRacer;
        UE_LOG(LogTemp, Log, TEXT("SpeedReductionPickup: Reduced AI speed from %f to %f"), OriginalSpeed, AIRacer->MaxSpeed);
    }

    // Set timer to reset speed after effect duration
    if (AffectedPlayer || AffectedAIRacer)
    {
        GetWorldTimerManager().SetTimer(ResetTimerHandle, this, &ASpeedReductionPickup::ResetSpeed, EffectDuration, false);
    }
}

void ASpeedReductionPickup::ResetSpeed()
{
    // Reset player speed to original value
    if (AffectedPlayer)
    {
        AffectedPlayer->SetSpeed(OriginalSpeed);
        UE_LOG(LogTemp, Log, TEXT("SpeedReductionPickup: Reset Player speed to %f"), OriginalSpeed);
        AffectedPlayer = nullptr;
    }
    // Reset AI speed to original value
    else if (AffectedAIRacer)
    {
        AffectedAIRacer->MaxSpeed = OriginalSpeed;
        UE_LOG(LogTemp, Log, TEXT("SpeedReductionPickup: Reset AI speed to %f"), OriginalSpeed);
        AffectedAIRacer = nullptr;
    }
}