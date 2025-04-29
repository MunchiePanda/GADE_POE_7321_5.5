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
	TArray<AActor*> FoundRacers; // Array to store found racers
	UGameplayStatics::GetAllActorsOfClass(Spectator->GetWorld(), AAIRacer::StaticClass(), FoundRacers); // Get all actors of class AAIRacer

	for (AActor* Actor : FoundRacers) // Iterate through all found racers
    {
		AAIRacer* Racer = Cast<AAIRacer>(Actor); // Cast the actor to AAIRacer
		// Check if the cast was successful
        if (Racer) 
        {
			float Distance = FVector::Dist(Spectator->GetActorLocation(), Racer->GetActorLocation()); // Calculate the distance between the spectator and the racer
            if (Distance <= Spectator->ReactionDistance) 
			{ // If the distance is within the reaction distance
                Spectator->Cheer();
                break;
            }
        }
    }
}

void UIdleState::ExitState(ASpectator* Spectator) // Called when exiting the Idle state
{
	UE_LOG(LogTemp, Log, TEXT("Spectator: Exiting Idle state")); // Log the exit of the Idle state
}