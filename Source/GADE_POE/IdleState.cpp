#include "IdleState.h"
#include "Spectator.h"
#include "Kismet/GameplayStatics.h"
#include "AIRacer.h"

void UIdleState::EnterState(ASpectator* Spectator)
{
    UE_LOG(LogTemp, Log, TEXT("Spectator: Entering Idle state"));
    Spectator->CurrentStateName = GetStateName();
}

void UIdleState::UpdateState(ASpectator* Spectator, float DeltaTime)
{
    TArray<AActor*> FoundRacers;
    UGameplayStatics::GetAllActorsOfClass(Spectator->GetWorld(), AAIRacer::StaticClass(), FoundRacers);

    for (AActor* Actor : FoundRacers)
    {
        AAIRacer* Racer = Cast<AAIRacer>(Actor);
        if (Racer)
        {
            float Distance = FVector::Dist(Spectator->GetActorLocation(), Racer->GetActorLocation());
            if (Distance <= Spectator->ReactionDistance)
            {
                Spectator->Cheer();
                break;
            }
        }
    }
}

void UIdleState::ExitState(ASpectator* Spectator)
{
    UE_LOG(LogTemp, Log, TEXT("Spectator: Exiting Idle state"));
}