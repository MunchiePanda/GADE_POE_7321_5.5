#include "DisappointedState.h"
#include "Spectator.h"

void UDisappointedState::EnterState(ASpectator* Spectator)
{
    UE_LOG(LogTemp, Log, TEXT("Spectator: Entering Disappointed state"));
    Spectator->CurrentStateName = GetStateName();
}

void UDisappointedState::UpdateState(ASpectator* Spectator, float DeltaTime)
{
    if (Spectator->StateTimer >= DisappointedDuration)
    {
        Spectator->BeIdle();
    }
}

void UDisappointedState::ExitState(ASpectator* Spectator)
{
    UE_LOG(LogTemp, Log, TEXT("Spectator: Exiting Disappointed state"));
}