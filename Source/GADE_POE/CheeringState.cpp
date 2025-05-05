#include "CheeringState.h"
#include "Spectator.h"

void UCheeringState::EnterState(ASpectator* Spectator)
{
    UE_LOG(LogTemp, Log, TEXT("Spectator: Entering Cheering state"));
    Spectator->CurrentStateName = GetStateName();
    StateTimer = 0.0f;
}

void UCheeringState::UpdateState(ASpectator* Spectator, float DeltaTime)
{
    StateTimer += DeltaTime; // Keep timer for potential use, but no automatic transition
    // Remove: if (StateTimer >= CheerDuration) { Spectator->BeIdle(); }
}

void UCheeringState::ExitState(ASpectator* Spectator)
{
    UE_LOG(LogTemp, Log, TEXT("Spectator: Exiting Cheering state"));
}