// BeginnerRaceGameState.cpp
#include "BiginnerRaceGameState.h"
#include "Kismet/GameplayStatics.h"
#include "WaypointManager.h"
#include "AdvancedRaceManager.h"

ABeginnerRaceGameState::ABeginnerRaceGameState()
{
    TotalLaps = 2;
    TotalWaypoints = 0;
    bRaceFinished = false;
}

void ABeginnerRaceGameState::BeginPlay()
{
    Super::BeginPlay();

    AAdvancedRaceManager* AdvancedManager = Cast<AAdvancedRaceManager>(
        UGameplayStatics::GetActorOfClass(GetWorld(), AAdvancedRaceManager::StaticClass()));
    if (AdvancedManager)
    {
        AdvancedManager->CollectWaypoints(); // Ensure waypoints are collected
        if (AdvancedManager->Waypoints.Num() > 0)
        {
            TotalWaypoints = AdvancedManager->Waypoints.Num();
            UE_LOG(LogTemp, Log, TEXT("BeginnerRaceGameState: Total waypoints set to %d from AdvancedRaceManager"), TotalWaypoints);
            return;
        }
    }

    AWaypointManager* WaypointManager = Cast<AWaypointManager>(
        UGameplayStatics::GetActorOfClass(GetWorld(), AWaypointManager::StaticClass()));
    if (WaypointManager && WaypointManager->Waypoints.Num() > 0)
    {
        TotalWaypoints = WaypointManager->Waypoints.Num();
        UE_LOG(LogTemp, Log, TEXT("BeginnerRaceGameState: Total waypoints set to %d from WaypointManager"), TotalWaypoints);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("BeginnerRaceGameState: No waypoint source found! Ensure AdvancedRaceManager with waypoints in Advanced Race or WaypointManager in Beginner Race."));
    }
}

void ABeginnerRaceGameState::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateLeaderboard();

    bool bAllFinished = true;
    for (const FRacerLeaderboardEntry& Entry : Leaderboard)
    {
        if (Entry.Lap < TotalLaps)
        {
            bAllFinished = false;
            break;
        }
    }
    if (bAllFinished && !bRaceFinished)
    {
        bRaceFinished = true;
        UE_LOG(LogTemp, Log, TEXT("BeginnerRaceGameState: Race finished!"));
    }
}

void ABeginnerRaceGameState::RegisterRacer(AActor* Racer)
{
    if (!Racer) return;

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
    if (!Racer)
    {
        UE_LOG(LogTemp, Error, TEXT("BeginnerRaceGameState: Tried to update progress for null racer"));
        return;
    }

    // Validate the waypoint index
    if (WaypointIndex < 0)
    {
        UE_LOG(LogTemp, Error, TEXT("BeginnerRaceGameState: Invalid waypoint index %d for racer %s"), 
            WaypointIndex, *Racer->GetName());
        return;
    }

    // Find and update the racer's entry
    bool bFound = false;
    for (FRacerLeaderboardEntry& Entry : Leaderboard)
    {
        if (Entry.Racer == Racer)
        {
            // Always update if the lap count has increased
            if (Lap > Entry.Lap)
            {
                Entry.Lap = Lap;
                Entry.WaypointIndex = WaypointIndex;
                bFound = true;
                UE_LOG(LogTemp, Warning, TEXT("BeginnerRaceGameState: Updated lap progress for %s: Lap %d, WaypointIndex %d"), 
                    *Racer->GetName(), Lap, WaypointIndex);
            }
            // If on the same lap, only update if waypoint index has increased
            else if (Lap == Entry.Lap && WaypointIndex > Entry.WaypointIndex)
            {
                Entry.WaypointIndex = WaypointIndex;
                bFound = true;
                UE_LOG(LogTemp, Warning, TEXT("BeginnerRaceGameState: Updated waypoint progress for %s: Lap %d, WaypointIndex %d"), 
                    *Racer->GetName(), Lap, WaypointIndex);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("BeginnerRaceGameState: Ignored invalid progress update for %s: Current(Lap:%d, WP:%d) New(Lap:%d, WP:%d)"), 
                    *Racer->GetName(), Entry.Lap, Entry.WaypointIndex, Lap, WaypointIndex);
            }
            break;
        }
    }

    if (!bFound)
    {
        UE_LOG(LogTemp, Error, TEXT("BeginnerRaceGameState: Could not find leaderboard entry for racer %s"), 
            *Racer->GetName());
        return;
    }

    // Update the leaderboard positions
    UpdateLeaderboard();
}

TArray<FRacerLeaderboardEntry> ABeginnerRaceGameState::GetLeaderboard() const
{
    return Leaderboard;
}

void ABeginnerRaceGameState::UpdateLeaderboard()
{
    // Sort based on lap count first, then waypoint index
    Leaderboard.Sort([](const FRacerLeaderboardEntry& A, const FRacerLeaderboardEntry& B) {
        // First compare laps
        if (A.Lap != B.Lap)
        {
            return A.Lap > B.Lap;
        }
        // If same lap, compare waypoint index
        return A.WaypointIndex > B.WaypointIndex;
    });

    // Update placements and log detailed progress
    for (int32 i = 0; i < Leaderboard.Num(); i++)
    {
        Leaderboard[i].Placement = i + 1;
        UE_LOG(LogTemp, Warning, TEXT("BeginnerRaceGameState: Racer %s - Lap: %d, Waypoint: %d, Position: %d"), 
            *Leaderboard[i].RacerName, 
            Leaderboard[i].Lap,
            Leaderboard[i].WaypointIndex,
            Leaderboard[i].Placement);
    }
}