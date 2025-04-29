#include "BeginnerRace_GMB.h"
#include "Spectator.h"
#include "TimerManager.h"
#include "EngineUtils.h" // For TActorIterator
ABeginnerRace_GMB::ABeginnerRace_GMB()
{
}

void ABeginnerRace_GMB::BeginPlay()
{
    Super::BeginPlay();

    // Find all spectators in the level
    for (TActorIterator<ASpectator> It(GetWorld()); It; ++It)
    {
        Spectators.Add(*It);
    }

    // Test Disappointed state after 10 seconds (simulating a race event)
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
        {
            for (ASpectator* Spectator : Spectators)
            {
                if (Spectator)
                {
                    Spectator->BeDisappointed();
                }
            }
        }, 10.0f, false);
}