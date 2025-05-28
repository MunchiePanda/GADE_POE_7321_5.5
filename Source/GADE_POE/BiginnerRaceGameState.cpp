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
    for (FRacerLeaderboardEntry& Entry : Leaderboard)
    {
        if (Entry.Racer == Racer)
        {
            Entry.Lap = Lap;
            Entry.WaypointIndex = WaypointIndex;
            UE_LOG(LogTemp, Log, TEXT("BeginnerRaceGameState: Updated progress for %s: Lap %d, WaypointIndex %d"), *Racer->GetName(), Lap, WaypointIndex);
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