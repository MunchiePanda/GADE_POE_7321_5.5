#include "CheeringState.h"
#include "Spectator.h"


/// Sets default values for this component's properties
void UCheeringState::EnterState(ASpectator* Spectator) 
{
	UE_LOG(LogTemp, Log, TEXT("Spectator: Entering Cheering state")); // Log the state change
	Spectator->CurrentStateName = GetStateName(); // Set the current state name
}

void UCheeringState::UpdateState(ASpectator* Spectator, float DeltaTime) 
{
	// Update the state timer
    if (Spectator->StateTimer >= CheerDuration)
    {
		Spectator->BeIdle(); // Transition to idle state after cheering
    }
}


/// Called when the state is exited
void UCheeringState::ExitState(ASpectator* Spectator)
{
	UE_LOG(LogTemp, Log, TEXT("Spectator: Exiting Cheering state")); // Log the state change
}