#include "SpeedBoostPickup.h"
#include "PlayerHamster.h"
#include "AIRacer.h"
#include "AIRacerContoller.h"
#include "Kismet/GameplayStatics.h"

void ASpeedBoostPickup::ApplyEffect(AActor* Racer)
{
    // Reset affected actors and original speed
    AffectedPlayer = nullptr;
    AffectedAIRacer = nullptr;
    OriginalSpeed = 0.0f;

    // Handle player character speed boost
    if (APlayerHamster* Player = Cast<APlayerHamster>(Racer))
    {
        // Store original speed and apply boost
        OriginalSpeed = Player->CurrentSpeed;
        Player->CurrentSpeed *= SpeedMultiplier;
        AffectedPlayer = Player;
        UE_LOG(LogTemp, Log, TEXT("SpeedBoostPickup: Boosted Player speed from %f to %f"), OriginalSpeed, Player->CurrentSpeed);
    }
    // Handle AI character speed boost
    else if (AAIRacer* AIRacer = Cast<AAIRacer>(Racer))
    {
        // Store original speed and apply boost
        OriginalSpeed = AIRacer->MaxSpeed;
        AIRacer->MaxSpeed *= SpeedMultiplier;
        AffectedAIRacer = AIRacer;
        UE_LOG(LogTemp, Log, TEXT("SpeedBoostPickup: Boosted AI speed from %f to %f"), OriginalSpeed, AIRacer->MaxSpeed);
    }

    // Set timer to reset speed after effect duration
    if (AffectedPlayer || AffectedAIRacer)
    {
        GetWorldTimerManager().SetTimer(ResetTimerHandle, this, &ASpeedBoostPickup::ResetSpeed, EffectDuration, false);
    }
}

void ASpeedBoostPickup::ResetSpeed()
{
    // Reset player speed to original value
    if (AffectedPlayer)
    {
        AffectedPlayer->CurrentSpeed = OriginalSpeed;
        UE_LOG(LogTemp, Log, TEXT("SpeedBoostPickup: Reset Player speed to %f"), OriginalSpeed);
        AffectedPlayer = nullptr;
    }
    // Reset AI speed to original value
    else if (AffectedAIRacer)
    {
        AffectedAIRacer->MaxSpeed = OriginalSpeed;
        UE_LOG(LogTemp, Log, TEXT("SpeedBoostPickup: Reset AI speed to %f"), OriginalSpeed);
        AffectedAIRacer = nullptr;
    }
}