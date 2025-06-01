#include "AIRacerContoller.h"
#include "Kismet/GameplayStatics.h"
#include "WaypointManager.h"
#include "CustomLinkedList.h"
#include "Graph.h"
#include "NavigationSystem.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "AIRacer.h"
#include "BiginnerRaceGameState.h"
#include "AdvancedRaceManager.h"
#include "DrawDebugHelpers.h"

AAIRacerContoller::AAIRacerContoller()
{
    PrimaryActorTick.bCanEverTick = true;
    bInitialized = false;
    bUseGraphNavigation = true;
}

void AAIRacerContoller::BeginPlay()
{
    Super::BeginPlay();

    GameState = Cast<ABeginnerRaceGameState>(GetWorld()->GetGameState());
    if (!GameState)
    {
        UE_LOG(LogTemp, Error, TEXT("AIRacerController %s: Failed to find BeginnerRaceGameState."), *GetName());
        return;
    }

    DetermineNavigationType();

    if (bUseGraphNavigation)
    {
        AdvancedRaceManager = Cast<AAdvancedRaceManager>(
            UGameplayStatics::GetActorOfClass(GetWorld(), AAdvancedRaceManager::StaticClass()));
        if (AdvancedRaceManager)
        {
            Graph = AdvancedRaceManager->GetGraph();
            if (!Graph)
            {
                UE_LOG(LogTemp, Error, TEXT("AIRacerController %s: Graph is null in AdvancedRaceManager."), *GetName());
                return;
            }
            if (AdvancedRaceManager->Waypoints.Num() == 0)
            {
                AdvancedRaceManager->CollectWaypoints();
            }
            if (AdvancedRaceManager->Waypoints.Num() > 0)
            {
                CurrentWaypoint = AdvancedRaceManager->Waypoints[0];
                UE_LOG(LogTemp, Log, TEXT("AIRacerController %s: Initialized CurrentWaypoint to %s (index 0)"),
                    *GetName(), *CurrentWaypoint->GetName());
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("AIRacerController %s: No waypoints in AdvancedRaceManager after collection."), *GetName());
                return;
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("AIRacerController %s: No AdvancedRaceManager found for graph navigation."), *GetName());
            return;
        }
    }
    else
    {
        WaypointManager = Cast<AWaypointManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AWaypointManager::StaticClass()));
        if (!WaypointManager || !WaypointManager->WaypointList)
        {
            UE_LOG(LogTemp, Error, TEXT("AIRacerController %s: Missing WaypointManager or LinkedList."), *GetName());
            return;
        }

        LinkedList = WaypointManager->WaypointList;
        CurrentWaypoint = LinkedList->GetFirst();

        if (!CurrentWaypoint)
        {
            UE_LOG(LogTemp, Error, TEXT("AIRacerController %s: No valid first waypoint."), *GetName());
            return;
        }
    }

    // Snap to first waypoint
    AAIRacer* Racer = Cast<AAIRacer>(GetPawn());
    if (Racer && CurrentWaypoint)
    {
        FVector WaypointLocation = CurrentWaypoint->GetActorLocation();
        Racer->SetActorLocation(WaypointLocation + FVector(0, 0, 10.0f), false);
        UE_LOG(LogTemp, Log, TEXT("AIRacerController %s: Snapped to first waypoint at %s"), *GetName(), *WaypointLocation.ToString());
    }

    if (GetPawn())
    {
        bInitialized = true;
        UE_LOG(LogTemp, Log, TEXT("AIRacerController %s: Pawn possessed, initializing navigation."), *GetName());
        GetWorld()->GetTimerManager().SetTimer(InitialMoveTimerHandle, this, &AAIRacerContoller::DelayedMoveToCurrentWaypoint, 0.5f, false);
    }
}

void AAIRacerContoller::InitializeGraph(AGraph* InGraph)
{
    Graph = InGraph;
    bUseGraphNavigation = true;

    if (Graph && AdvancedRaceManager)
    {
        if (AdvancedRaceManager->Waypoints.Num() > 0)
        {
            CurrentWaypoint = AdvancedRaceManager->Waypoints[0];
            bInitialized = false;
            UE_LOG(LogTemp, Log, TEXT("AIRacerController %s: Initialized Graph with CurrentWaypoint %s (index 0)"),
                *GetName(), *CurrentWaypoint->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("AIRacerController %s: No waypoints in AdvancedRaceManager."), *GetName());
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AIRacerController %s: Graph or AdvancedRaceManager is null."), *GetName());
    }
}

void AAIRacerContoller::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bInitialized && GetPawn())
    {
        bInitialized = true;
        UE_LOG(LogTemp, Log, TEXT("AIRacerController %s: Pawn possessed, initializing navigation."), *GetName());
        GetWorld()->GetTimerManager().SetTimer(InitialMoveTimerHandle, this, &AAIRacerContoller::DelayedMoveToCurrentWaypoint, 0.5f, false);
    }
}

void AAIRacerContoller::DelayedMoveToCurrentWaypoint()
{
    MoveToCurrentWaypoint();
}

void AAIRacerContoller::OnWaypointReached(AActor* ReachedWaypoint)
{
    if (!ReachedWaypoint || ReachedWaypoint != CurrentWaypoint)
        return;

    AAIRacer* Racer = Cast<AAIRacer>(GetPawn());
    if (Racer)
    {
        Racer->WaypointsPassed++;
        if (bUseGraphNavigation && AdvancedRaceManager)
        {
            int32 ReachedIndex = AdvancedRaceManager->Waypoints.Find(Cast<AWaypoint>(ReachedWaypoint));
            UE_LOG(LogTemp, Log, TEXT("AIRacerController %s: Reached waypoint %s (index %d)"),
                *GetName(), *ReachedWaypoint->GetName(), ReachedIndex);

            TArray<AActor*> Neighbors = Graph->GetNeighbors(ReachedWaypoint);
            if (Neighbors.Num() > 0)
            {
                int32 RandomIndex = FMath::RandRange(0, Neighbors.Num() - 1);
                CurrentWaypoint = Neighbors[RandomIndex];
                int32 NextIndex = AdvancedRaceManager->Waypoints.Find(Cast<AWaypoint>(CurrentWaypoint));
                UE_LOG(LogTemp, Log, TEXT("AIRacerController %s: Randomly selected next waypoint %s (index %d) from %d neighbors"),
                    *GetName(), *CurrentWaypoint->GetName(), NextIndex, Neighbors.Num());
            }
            else
            {
                CurrentWaypoint = AdvancedRaceManager->Waypoints[0];
                int32 FallbackIndex = 0;
                UE_LOG(LogTemp, Log, TEXT("AIRacerController %s: No neighbors for %s, falling back to waypoint %s (index %d)"),
                    *GetName(), *ReachedWaypoint->GetName(), *CurrentWaypoint->GetName(), FallbackIndex);
            }

            if (ReachedIndex == 0 && Racer->WaypointsPassed > 1)
            {
                Racer->LapCount++;
                Racer->WaypointsPassed = 1;
                UE_LOG(LogTemp, Log, TEXT("AIRacerController %s: Completed lap %d"), *GetName(), Racer->LapCount);
            }
        }
        else
        {
            int32 WaypointIndex = Racer->WaypointsPassed;
            if (GameState && GameState->TotalWaypoints > 0 && Racer->WaypointsPassed >= GameState->TotalWaypoints)
            {
                Racer->LapCount++;
                Racer->WaypointsPassed = 0;
                WaypointIndex = 0;
                UE_LOG(LogTemp, Log, TEXT("AIRacerController %s: Completed lap %d"), *GetName(), Racer->LapCount);
            }

            CurrentWaypoint = LinkedList->GetNext(ReachedWaypoint);
            if (!CurrentWaypoint)
            {
                CurrentWaypoint = LinkedList->GetFirst();
                UE_LOG(LogTemp, Log, TEXT("AIRacerController %s: Looped back to first waypoint"), *GetName());
            }
            UE_LOG(LogTemp, Log, TEXT("AIRacerController %s: Moved to next waypoint %s (index %d)"),
                *GetName(), *CurrentWaypoint->GetName(), WaypointIndex);
        }

        if (GameState)
        {
            GameState->UpdateRacerProgress(Racer, Racer->LapCount, Racer->WaypointsPassed);
        }
    }

    MoveToCurrentWaypoint();
}

void AAIRacerContoller::DetermineNavigationType()
{
    FString CurrentLevelName = GetWorld()->GetMapName();
    CurrentLevelName.RemoveFromStart("UEDPIE_0_");

    if (CurrentLevelName == "BeginnerMap")
    {
        bUseGraphNavigation = false;
        UE_LOG(LogTemp, Log, TEXT("AIRacerController %s: Using WaypointManager for BeginnerMap."), *GetName());
    }
    else if (CurrentLevelName == "AdvancedMap")
    {
        bUseGraphNavigation = true;
        UE_LOG(LogTemp, Log, TEXT("AIRacerController %s: Using AdvancedRaceManager for AdvancedMap."), *GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacerController %s: Unknown map '%s', defaulting to graph navigation."), *GetName(), *CurrentLevelName);
    }
}

void AAIRacerContoller::MoveToCurrentWaypoint()
{
    if (!CurrentWaypoint)
    {
        UE_LOG(LogTemp, Error, TEXT("AIRacerController %s: CurrentWaypoint is null."), *GetName());
        return;
    }

    AAIRacer* Racer = Cast<AAIRacer>(GetPawn());
    if (Racer && IsValid(CurrentWaypoint))
    {
        UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
        if (NavSystem)
        {
            EPathFollowingRequestResult::Type Result = MoveToLocation(CurrentWaypoint->GetActorLocation(), 200.0f);
            switch (Result)
            {
            case EPathFollowingRequestResult::RequestSuccessful:
                UE_LOG(LogTemp, Log, TEXT("AIRacerController %s: Moving to waypoint %s at %s"),
                    *GetName(), *CurrentWaypoint->GetName(), *CurrentWaypoint->GetActorLocation().ToString());
                break;
            case EPathFollowingRequestResult::Failed:
                UE_LOG(LogTemp, Warning, TEXT("AIRacerController %s: Failed to move to waypoint %s"),
                    *GetName(), *CurrentWaypoint->GetName());
                break;
                // Handle other enum values as needed
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("AIRacerController %s: Navigation system not available."), *GetName());
        }
    }
}