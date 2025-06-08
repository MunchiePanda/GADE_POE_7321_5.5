#include "CheckpointManager.h"
#include "CheckpointActor.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "CheckpointRace_GMB.h"

// Sets default values
ACheckpointManager::ACheckpointManager()
{
    PrimaryActorTick.bCanEverTick = true;
    RemainingTime = InitialTime; // Initialize remaining time
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

    GetNextCheckpoint();
}

// Called every frame
void ACheckpointManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update the remaining time
    RemainingTime -= DeltaTime;

    // Check if the timer has expired
    if (RemainingTime <= 0.0f)
    {
        HandleTimerExpiry();
    }
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
    // Prevent multiple triggers for the same checkpoint
    if (bCheckpointCleared) return;
    bCheckpointCleared = true;

    ACheckpointActor* ReachedCheckpoint;
    if (CheckpointStack.Pop(ReachedCheckpoint))
    {
        // Log checkpoint progress
        UE_LOG(LogTemp, Warning, TEXT("Checkpoint Passed: %s"), *ReachedCheckpoint->GetName());

        if (IsValid(ReachedCheckpoint))
        { 
            // Update checkpoint visual state
            ReachedCheckpoint->SetCheckpointState(true, false);
        }

        // Play checkpoint reached sound effect
        if (CheckpointReachedSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, CheckpointReachedSound, ReachedCheckpoint->GetActorLocation());
        }

        // Add bonus time for reaching checkpoint
        RemainingTime += TimePerCheckpoint;
    }

    // Check if all checkpoints in current lap are cleared
    if (CheckpointStack.IsEmpty())
    {
        if (CurrentLap < TotalLaps)
        {
            // Start new lap
            CurrentLap++;
            ResetCheckpoints();
            UE_LOG(LogTemp, Warning, TEXT("Lap %d/%d Completed!"), CurrentLap - 1, TotalLaps);
        }
        else
        {
            // Race finished
            UE_LOG(LogTemp, Warning, TEXT("Race Finished!"));
            ACheckpointRace_GMB* GameMode = Cast<ACheckpointRace_GMB>(UGameplayStatics::GetGameMode(GetWorld()));
            if (GameMode)
            {
                GameMode->CheckRaceStatus();
            }
        }
    }
    else
    {
        // Update next checkpoint indicator
        GetNextCheckpoint();
    }

    // Reset checkpoint cleared flag on next tick
    GetWorldTimerManager().SetTimerForNextTick([this]() { bCheckpointCleared = false; });
}

ACheckpointActor* ACheckpointManager::GetNextCheckpoint() // Get the next checkpoint
{
    ACheckpointActor* NextCheckpoint;
    if (CheckpointStack.Peek(NextCheckpoint) && IsValid(NextCheckpoint)) // Check if checkpoint is valid
    {
        NextCheckpoint->SetCheckpointState(false, true);
        return NextCheckpoint;
    }
    return nullptr;
}

void ACheckpointManager::ResetCheckpoints()
{
    UE_LOG(LogTemp, Warning, TEXT("Resetting Checkpoints for New Lap..."));

    for (int32 i = AllCheckpoints.Num() - 1; i >= 0; i--) // Push in reverse
    {
        if (IsValid(AllCheckpoints[i]))
        {
            CheckpointStack.Push(AllCheckpoints[i]);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Checkpoints Reset! Stack Size: %d"), CheckpointStack.Size());
}

void ACheckpointManager::DebugCheckpointStatus()
{
    if (CheckpointStack.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("No checkpoints left in stack!"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("----- Debugging Checkpoints -----"));

    CheckStackTemp<ACheckpointActor*> TempStack; // Use a temporary stack
    while (!CheckpointStack.IsEmpty())
    {
        ACheckpointActor* Checkpoint;
        if (CheckpointStack.Pop(Checkpoint) && IsValid(Checkpoint))
        {
            UE_LOG(LogTemp, Warning, TEXT("Checkpoint: %s"), *Checkpoint->GetName());
            DrawDebugSphere(GetWorld(), Checkpoint->GetActorLocation(), 50.0f, 12, FColor::Red, false, 5.0f);

            TempStack.Push(Checkpoint);
        }
    }

    // Restore stack order exactly as it was before
    while (!TempStack.IsEmpty())
    {
        ACheckpointActor* Checkpoint;
        TempStack.Pop(Checkpoint);
        CheckpointStack.Push(Checkpoint);
    }

    UE_LOG(LogTemp, Warning, TEXT("----- End Debug -----"));
}

// Get the number of checkpoints reached
int32 ACheckpointManager::GetRemainingCheckpoint() const
{
    return CheckpointStack.Size();
}

void ACheckpointManager::HandleTimerExpiry()
{
    UE_LOG(LogTemp, Warning, TEXT("Time's up!"));

    // Handle the timer expiry (e.g., end the race, show UI, etc.)
    ACheckpointRace_GMB* GameMode = Cast<ACheckpointRace_GMB>(UGameplayStatics::GetGameMode(GetWorld()));
    if (GameMode)
    {
        GameMode->CheckRaceStatus(); // Call the function
    }
    // Add additional logic here if needed
}

