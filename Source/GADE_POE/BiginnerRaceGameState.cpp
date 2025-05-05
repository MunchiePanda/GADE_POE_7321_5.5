#include "BiginnerRaceGameState.h"
#include "Kismet/GameplayStatics.h"
#include "WaypointManager.h"

ABeginnerRaceGameState::ABeginnerRaceGameState()
{
    TotalLaps = 2; // Set to desired number of laps
    TotalWaypoints = 0;
    bRaceFinished = false;
}

void ABeginnerRaceGameState::BeginPlay()
{
    Super::BeginPlay();

    // Get total waypoints from WaypointManager
    AWaypointManager* WaypointManager = Cast<AWaypointManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AWaypointManager::StaticClass()));
    if (WaypointManager && WaypointManager->Waypoints.Num() > 0)
    {
		TotalWaypoints = WaypointManager->Waypoints.Num(); // Get the number of waypoints from the WaypointManager
        UE_LOG(LogTemp, Log, TEXT("BeginnerRaceGameState: TotalWaypoints set to %d"), TotalWaypoints);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("BeginnerRaceGameState: WaypointManager not found or no waypoints!")); 
    }
}

void ABeginnerRaceGameState::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateLeaderboard();

    // Check if all racers have finished
    bool bAllFinished = true;
    for (const FRacerLeaderboardEntry& Entry : Leaderboard)
    {
        if (Entry.Lap < TotalLaps)
        {
            bAllFinished = false;
            break;
        }
    }
    if (bAllFinished && !bRaceFinished) // Only set bRaceFinished once all racers have finished
    {
        bRaceFinished = true;
        UE_LOG(LogTemp, Log, TEXT("BeginnerRaceGameState: Race finished!"));
    }
}

void ABeginnerRaceGameState::RegisterRacer(AActor* Racer) // Register the racer with the game state 
{
    if (!Racer) return;

    // Add the racer to the leaderboard 
    FRacerLeaderboardEntry Entry;
    Entry.Racer = Racer; 
    Entry.RacerName = Racer->GetName();
    Entry.Lap = 0;
    Entry.WaypointIndex = 0;
    Entry.Placement = 0;
    Leaderboard.Add(Entry);

    UE_LOG(LogTemp, Log, TEXT("BeginnerRaceGameState: Registered racer %s"), *Entry.RacerName);
}

void ABeginnerRaceGameState::UpdateRacerProgress(AActor* Racer, int32 Lap, int32 WaypointIndex)
{
    for (FRacerLeaderboardEntry& Entry : Leaderboard) // Loop through the leaderboard
    {
        // Update the racer's progress 
        if (Entry.Racer == Racer)
        {
            Entry.Lap = Lap;
            Entry.WaypointIndex = WaypointIndex;
            UE_LOG(LogTemp, Log, TEXT("BeginnerRaceGameState: Updated progress for %s: Lap %d, WaypointIndex %d"), *Racer->GetName(), Lap, WaypointIndex);
            break;
        }
    }
    UpdateLeaderboard(); // Update the leaderboard
}

TArray<FRacerLeaderboardEntry> ABeginnerRaceGameState::GetLeaderboard() const
{
	return Leaderboard; // Return the leaderboard
}

void ABeginnerRaceGameState::UpdateLeaderboard() // Update the leaderboard based on lap and waypoint index
{
	// Sort the leaderboard based on lap and waypoint index (this is what allows me to see if a character has completed a lap to correctly update positions)
    Leaderboard.Sort([](const FRacerLeaderboardEntry& A, const FRacerLeaderboardEntry& B) {
        if (A.Lap != B.Lap)
        {
            return A.Lap > B.Lap;
        }
        return A.WaypointIndex > B.WaypointIndex;
        });

    for (int32 i = 0; i < Leaderboard.Num(); i++)
    {
        Leaderboard[i].Placement = i + 1;
        UE_LOG(LogTemp, Log, TEXT("BeginnerRaceGameState: Updated placement for %s to %d"), *Leaderboard[i].RacerName, Leaderboard[i].Placement);
    }
}