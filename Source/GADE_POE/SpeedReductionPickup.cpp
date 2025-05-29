#include "SpeedReductionPickup.h"
#include "PlayerHamster.h"
#include "AIRacer.h"
#include "AIRacerContoller.h"

void ASpeedReductionPickup::ApplyEffect(AActor* Racer)
{
    if (APlayerHamster* Player = Cast<APlayerHamster>(Racer))
    {
        OriginalSpeed = Player->CurrentSpeed; // Initialize OriginalSpeed
        Player->CurrentSpeed *= SpeedMultiplier;
        GetWorldTimerManager().SetTimer(ResetTimerHandle, this, &ASpeedReductionPickup::ResetPlayerSpeed, EffectDuration, false);
    }
    else if (AAIRacer* AIRacer = Cast<AAIRacer>(Racer))
    {
        OriginalSpeed = AIRacer->MaxSpeed; // Initialize OriginalSpeed
        AIRacer->MaxSpeed *= SpeedMultiplier;
        LastAIRacer = AIRacer; // Track the affected AI racer
        GetWorldTimerManager().SetTimer(ResetTimerHandle, this, &ASpeedReductionPickup::ResetAISpeed, EffectDuration, false);
    }
}

void ASpeedReductionPickup::ResetPlayerSpeed()
{
    if (APlayerHamster* Player = Cast<APlayerHamster>(GetWorld()->GetFirstPlayerController()->GetPawn()))
    {
        Player->CurrentSpeed = OriginalSpeed;
    }
}

void ASpeedReductionPickup::ResetAISpeed()
{
    if (LastAIRacer)
    {
        LastAIRacer->MaxSpeed = OriginalSpeed;
        LastAIRacer = nullptr; // Clear the reference after reset
    }
}