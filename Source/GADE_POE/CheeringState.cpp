#include "CheeringState.h"
#include "Spectator.h"


void UCheeringState::EnterState(ASpectator* Spectator)
{
    UE_LOG(LogTemp, Log, TEXT("Spectator: Entering Cheering state"));
    Spectator->CurrentStateName = GetStateName();
}

void UCheeringState::UpdateState(ASpectator* Spectator, float DeltaTime)
{
    if (Spectator->StateTimer >= CheerDuration)
    {
        Spectator->BeIdle();
    }
}

void UCheeringState::ExitState(ASpectator* Spectator)
{
    UE_LOG(LogTemp, Log, TEXT("Spectator: Exiting Cheering state"));
}