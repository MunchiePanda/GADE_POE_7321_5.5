#include "DisappointedState.h"
#include "Spectator.h"

void UDisappointedState::EnterState(ASpectator* Spectator) 
{
	UE_LOG(LogTemp, Log, TEXT("Spectator: Entering Disappointed state")); // Log the state change
	Spectator->CurrentStateName = GetStateName(); // Set the current state name
}

void UDisappointedState::UpdateState(ASpectator* Spectator, float DeltaTime) // Update the state every frame
{
    if (Spectator->StateTimer >= DisappointedDuration)
    {
		Spectator->BeIdle(); // Transition back to idle state after the duration
    }
}

void UDisappointedState::ExitState(ASpectator* Spectator) // Exit the state
{
    UE_LOG(LogTemp, Log, TEXT("Spectator: Exiting Disappointed state"));
}