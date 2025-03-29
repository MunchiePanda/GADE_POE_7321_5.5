#include "CheckpointManager.h"
#include "CheckpointActor.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ACheckpointManager::ACheckpointManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts
void ACheckpointManager::BeginPlay()
{
	Super::BeginPlay(); 

	// Find all CheckpointActors
	TArray<AActor*> FoundCheckpoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACheckpointActor::StaticClass(), FoundCheckpoints);

	for (AActor* Actor : FoundCheckpoints)
	{
		ACheckpointActor* Checkpoint = Cast<ACheckpointActor>(Actor);
		if (Checkpoint)
		{
			AddCheckpoint(Checkpoint); // Add checkpoints to the stack
			AllCheckpoints.Add(Checkpoint); // Store for resetting
			
		}
	}

	
	UE_LOG(LogTemp, Warning, TEXT("Checkpoint Manager Initialized! Stack Size: %d"), CheckpointStack.Size()); // Log stack size
}


// Called every frame
void ACheckpointManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

void ACheckpointManager::AddCheckpoint(ACheckpointActor* Checkpoint) // Add checkpoints to the stack
{
	if (Checkpoint)
	{
		CheckpointStack.Push(Checkpoint);
		UE_LOG(LogTemp, Warning, TEXT("Checkpoint Added: %s"), *Checkpoint->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Tried to add a null checkpoint!"));
	}
}

void ACheckpointManager::PlayerReachedCheckpoint()
{
	if (bCheckpointCleared) return;
	bCheckpointCleared = true;

	ACheckpointActor* ReachedCheckpoint;
	if (CheckpointStack.Pop(ReachedCheckpoint))
	{
		UE_LOG(LogTemp, Warning, TEXT("Checkpoint Passed: %s"), *ReachedCheckpoint->GetName());
	}

	// If stack is empty, the player completed a lap
	if (CheckpointStack.IsEmpty())
	{
		if (CurrentLap < TotalLaps)
		{
			CurrentLap++;
			ResetCheckpoints(); // Refill the stack for next lap
			UE_LOG(LogTemp, Warning, TEXT("Lap %d/%d Completed!"), CurrentLap - 1, TotalLaps);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Race Finished!"));
			// You could add logic here to end the race, show UI, etc.
		}
	}

	GetWorldTimerManager().SetTimerForNextTick([this]() { bCheckpointCleared = false; });
}

ACheckpointActor* ACheckpointManager::GetNextCheckpoint() // Get the next checkpoint
{
	ACheckpointActor* NextCheckpoint;
	if (CheckpointStack.Peek(NextCheckpoint) && IsValid(NextCheckpoint)) // Check if checkpoint is valid
	{
		return NextCheckpoint;
	}
	return nullptr;
}

void ACheckpointManager::ResetCheckpoints()
{
	UE_LOG(LogTemp, Warning, TEXT("Resetting Checkpoints for New Lap..."));

	// Re-add all checkpoints
	for (ACheckpointActor* Checkpoint : AllCheckpoints)
	{
		if (IsValid(Checkpoint)) // Check if checkpoint is valid
		{
			CheckpointStack.Push(Checkpoint);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Checkpoints Reset! Stack Size: %d"), CheckpointStack.Size());
}

void ACheckpointManager::DebugCheckpointStatus() // Debug checkpoints
{
	if (CheckpointStack.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("No checkpoints left in stack!"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("----- Debugging Checkpoints -----"));

	TArray<ACheckpointActor*> TempStack; // Temporary stack
	while (!CheckpointStack.IsEmpty())
	{
		ACheckpointActor* Checkpoint;
		if (CheckpointStack.Pop(Checkpoint) && IsValid(Checkpoint)) // Check if checkpoint is valid
		{
			UE_LOG(LogTemp, Warning, TEXT("Checkpoint: %s"), *Checkpoint->GetName()); // Log checkpoint name 
			DrawDebugSphere(GetWorld(), Checkpoint->GetActorLocation(), 50.0f, 12, FColor::Red, false, 5.0f); // Draw checkpoint sphere

			// Save for restoring the stack order
			TempStack.Add(Checkpoint);
		}
	}

	// Restore stack order
	for (int i = TempStack.Num() - 1; i >= 0; i--)
	{
		CheckpointStack.Push(TempStack[i]);
	}

	UE_LOG(LogTemp, Warning, TEXT("----- End Debug -----"));
}
// Get the number of checkpoints reached
int32 ACheckpointManager::GetRemainingCheckpoint() const
{
	return  CheckpointStack.Size();
}
