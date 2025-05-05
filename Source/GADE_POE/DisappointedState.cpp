#include "DisappointedState.h"
#include "Spectator.h"

void UDisappointedState::EnterState(ASpectator* Spectator)
{
    UE_LOG(LogTemp, Log, TEXT("Spectator: Entering Disappointed state")); 
    Spectator->CurrentStateName = GetStateName();
    StateTimer = 0.0f; // Reset timer on entering state
}

void UDisappointedState::UpdateState(ASpectator* Spectator, float DeltaTime) // Update the state 
{
    StateTimer += DeltaTime; // Increment timer
}

void UDisappointedState::ExitState(ASpectator* Spectator) // Exit the disappointed state 
{
    UE_LOG(LogTemp, Log, TEXT("Spectator: Exiting Disappointed state"));
}