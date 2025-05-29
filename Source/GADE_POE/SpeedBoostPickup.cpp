#include "SpeedBoostPickup.h"
#include "PlayerHamster.h"
#include "AIRacer.h"
#include "AIRacerContoller.h"

void ASpeedBoostPickup::ApplyEffect(AActor* Racer)
{
    if (APlayerHamster* Player = Cast<APlayerHamster>(Racer))
    {
        OriginalSpeed = Player->CurrentSpeed; // Initialize OriginalSpeed
        Player->CurrentSpeed *= SpeedMultiplier;
        GetWorldTimerManager().SetTimer(ResetTimerHandle, this, &ASpeedBoostPickup::ResetPlayerSpeed, EffectDuration, false);
    }
    else if (AAIRacer* AIRacer = Cast<AAIRacer>(Racer))
    {
        OriginalSpeed = AIRacer->MaxSpeed; // Initialize OriginalSpeed
        AIRacer->MaxSpeed *= SpeedMultiplier;
        LastAIRacer = AIRacer; // Track the affected AI racer
        GetWorldTimerManager().SetTimer(ResetTimerHandle, this, &ASpeedBoostPickup::ResetAISpeed, EffectDuration, false);
    }
}

void ASpeedBoostPickup::ResetPlayerSpeed()
{
    if (APlayerHamster* Player = Cast<APlayerHamster>(GetWorld()->GetFirstPlayerController()->GetPawn()))
    {
        Player->CurrentSpeed = OriginalSpeed;
    }
}

void ASpeedBoostPickup::ResetAISpeed()
{
    if (LastAIRacer)
    {
        LastAIRacer->MaxSpeed = OriginalSpeed;
        LastAIRacer = nullptr; // Clear the reference after reset
    }
}