#include "SpeedReductionPickup.h"
#include "PlayerHamster.h"
#include "AIRacer.h"
#include "AIRacerContoller.h"
#include "Kismet/GameplayStatics.h"

void ASpeedReductionPickup::ApplyEffect(AActor* Racer)
{
    AffectedPlayer = nullptr;
    AffectedAIRacer = nullptr;
    OriginalSpeed = 0.0f;

    if (APlayerHamster* Player = Cast<APlayerHamster>(Racer))
    {
        OriginalSpeed = Player->CurrentSpeed;
        Player->CurrentSpeed *= SpeedMultiplier;
        AffectedPlayer = Player;
        UE_LOG(LogTemp, Log, TEXT("SpeedReductionPickup: Reduced Player speed from %f to %f"), OriginalSpeed, Player->CurrentSpeed);
    }
    else if (AAIRacer* AIRacer = Cast<AAIRacer>(Racer))
    {
        OriginalSpeed = AIRacer->MaxSpeed;
        AIRacer->MaxSpeed *= SpeedMultiplier;
        AffectedAIRacer = AIRacer;
        UE_LOG(LogTemp, Log, TEXT("SpeedReductionPickup: Reduced AI speed from %f to %f"), OriginalSpeed, AIRacer->MaxSpeed);
    }

    if (AffectedPlayer || AffectedAIRacer)
    {
        GetWorldTimerManager().SetTimer(ResetTimerHandle, this, &ASpeedReductionPickup::ResetSpeed, EffectDuration, false);
    }
}

void ASpeedReductionPickup::ResetSpeed()
{
    if (AffectedPlayer)
    {
        AffectedPlayer->CurrentSpeed = OriginalSpeed;
        UE_LOG(LogTemp, Log, TEXT("SpeedReductionPickup: Reset Player speed to %f"), OriginalSpeed);
        AffectedPlayer = nullptr;
    }
    else if (AffectedAIRacer)
    {
        AffectedAIRacer->MaxSpeed = OriginalSpeed;
        UE_LOG(LogTemp, Log, TEXT("SpeedReductionPickup: Reset AI speed to %f"), OriginalSpeed);
        AffectedAIRacer = nullptr;
    }
}