#include "IdleState.h"
#include "Spectator.h"
#include "Kismet/GameplayStatics.h"
#include "AIRacer.h"

void UIdleState::EnterState(ASpectator* Spectator) // Called when entering the Idle state
{
    UE_LOG(LogTemp, Log, TEXT("Spectator: Entering Idle state"));
	Spectator->CurrentStateName = GetStateName(); // Set the current state name
}

void UIdleState::UpdateState(ASpectator* Spectator, float DeltaTime)
{
	// No logic here; transitions are handled by SwitchToNextState
}

void UIdleState::ExitState(ASpectator* Spectator) // Called when exiting the Idle state
{
	UE_LOG(LogTemp, Log, TEXT("Spectator: Exiting Idle state")); // Log the exit of the Idle state
}