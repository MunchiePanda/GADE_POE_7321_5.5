// Fill out your copyright notice in the Description page of Project Settings.


#include "BiginnerRaceGameState.h"
#include "Kismet/GameplayStatics.h"
#include "WaypointManager.h"


ABeginnerRaceGameState::ABeginnerRaceGameState()
{
    TotalLaps = 3; // Set to desired number of laps
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
        TotalWaypoints = WaypointManager->Waypoints.Num();
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
    for (FRacerLeaderboardEntry& Entry : Leaderboard)
    {
        if (Entry.Racer == Racer)
        {
            Entry.Lap = Lap;
            Entry.WaypointIndex = WaypointIndex;
            break;
        }
    }
    UpdateLeaderboard();
}

TArray<FRacerLeaderboardEntry> ABeginnerRaceGameState::GetLeaderboard() const
{
    return Leaderboard;
}

void ABeginnerRaceGameState::UpdateLeaderboard()
{
    // Sort by lap (descending), then waypoint index (descending)
    Leaderboard.Sort([](const FRacerLeaderboardEntry& A, const FRacerLeaderboardEntry& B) {
        if (A.Lap != B.Lap)
        {
            return A.Lap > B.Lap;
        }
        return A.WaypointIndex > B.WaypointIndex;
        });

    // Assign placements
    for (int32 i = 0; i < Leaderboard.Num(); i++)
    {
        Leaderboard[i].Placement = i + 1;
    }
}