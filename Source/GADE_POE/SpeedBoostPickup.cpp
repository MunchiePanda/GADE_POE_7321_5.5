#include "SpeedBoostPickup.h"
#include "PlayerHamster.h"
#include "AIRacer.h"
#include "AIRacerContoller.h"
#include "Kismet/GameplayStatics.h"

void ASpeedBoostPickup::ApplyEffect(AActor* Actor)
{
    APlayerHamster* PlayerHamster = Cast<APlayerHamster>(Actor);
    if (PlayerHamster)
    {
        float CurrentSpeed = PlayerHamster->GetSpeed();
        float MaxSpeed = PlayerHamster->GetMaxSpeed();
        float NewSpeed = FMath::Min(CurrentSpeed * SpeedMultiplier, MaxSpeed);
        PlayerHamster->SetSpeed(NewSpeed);
        UE_LOG(LogTemp, Log, TEXT("SpeedBoostPickup: Boosted speed from %f to %f"), CurrentSpeed, NewSpeed);
    }
}

void ASpeedBoostPickup::ResetSpeed()
{
    // Reset player speed to original value
    if (AffectedPlayer)
    {
        AffectedPlayer->SetSpeed(OriginalSpeed);
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