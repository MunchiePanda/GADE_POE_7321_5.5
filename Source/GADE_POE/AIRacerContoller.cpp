// AIRacerContoller.cpp
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
        UE_LOG(LogTemp, Error, TEXT("AIRacerContoller %s: Failed to find BeginnerRaceGameState."), *GetName());
        return;
    }

    AdvancedRaceManager = Cast<AAdvancedRaceManager>(
        UGameplayStatics::GetActorOfClass(GetWorld(), AAdvancedRaceManager::StaticClass()));
    if (AdvancedRaceManager)
    {
        Graph = AdvancedRaceManager->GetGraph();
        if (!Graph)
        {
            UE_LOG(LogTemp, Error, TEXT("AIRacerContoller %s: Graph is null in AdvancedRaceManager."), *GetName());
            return;
        }
        if (AdvancedRaceManager->Waypoints.Num() == 0)
        {
            AdvancedRaceManager->CollectWaypoints();
        }
        if (AdvancedRaceManager->Waypoints.Num() > 0)
        {
            CurrentWaypoint = AdvancedRaceManager->Waypoints[0];
            UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Initialized CurrentWaypoint to %s (index 0)"),
                *GetName(), *CurrentWaypoint->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("AIRacerContoller %s: No waypoints in AdvancedRaceManager."), *GetName());
            return;
        }
    }
    else if (!bUseGraphNavigation)
    {
        WaypointManager = Cast<AWaypointManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AWaypointManager::StaticClass()));
        if (!WaypointManager || !WaypointManager->WaypointList)
        {
            UE_LOG(LogTemp, Error, TEXT("AIRacerContoller %s: Missing WaypointManager or LinkedList."), *GetName());
            return;
        }

        LinkedList = WaypointManager->WaypointList;
        CurrentWaypoint = LinkedList->GetFirst();

        if (!CurrentWaypoint)
        {
            UE_LOG(LogTemp, Error, TEXT("AIRacerContoller %s: No valid first waypoint."), *GetName());
            return;
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AIRacerContoller %s: No AdvancedRaceManager found for graph navigation."), *GetName());
        return;
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
            UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Initialized Graph with CurrentWaypoint %s (index 0)"),
                *GetName(), *CurrentWaypoint->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("AIRacerContoller %s: No waypoints in AdvancedRaceManager."), *GetName());
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AIRacerContoller %s: Graph or AdvancedRaceManager is null."), *GetName());
    }
}

void AAIRacerContoller::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bInitialized && GetPawn())
    {
        bInitialized = true;
        UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Pawn possessed, initializing navigation."), *GetName());
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

    int32 ReachedIndex = AdvancedRaceManager->Waypoints.Find(Cast<AWaypoint>(ReachedWaypoint));
    UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Reached waypoint %s (index %d)"),
        *GetName(), *ReachedWaypoint->GetName(), ReachedIndex);

    AAIRacer* Racer = Cast<AAIRacer>(GetPawn());
    int32 WaypointIndex = 0;

    if (Racer)
    {
        Racer->WaypointsPassed++;
        if (bUseGraphNavigation && AdvancedRaceManager)
        {
            WaypointIndex = ReachedIndex;
            if (WaypointIndex == INDEX_NONE) WaypointIndex = 0;

            TArray<AActor*> Neighbors = Graph->GetNeighbors(ReachedWaypoint);
            FString NeighborNames = TEXT("None"), NeighborIndices = TEXT("");
            if (Neighbors.Num() > 0)
            {
                NeighborNames = TEXT(""); NeighborIndices = TEXT("");
                for (AActor* Neighbor : Neighbors)
                {
                    int32 NeighborIndex = AdvancedRaceManager->Waypoints.Find(Cast<AWaypoint>(Neighbor));
                    NeighborNames += Neighbor->GetName() + TEXT(", ");
                    NeighborIndices += FString::Printf(TEXT("%d, "), NeighborIndex);
                }
                NeighborNames.RemoveFromEnd(TEXT(", ")); NeighborIndices.RemoveFromEnd(TEXT(", "));
            }
            UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Waypoint %s (index %d) has %d neighbors: %s (indices: %s)"),
                *GetName(), *ReachedWaypoint->GetName(), ReachedIndex, Neighbors.Num(), *NeighborNames, *NeighborIndices);

            if (WaypointIndex == 0 && Racer->WaypointsPassed > 1)
            {
                Racer->LapCount++; Racer->WaypointsPassed = 1;
                UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Completed lap %d"), *GetName(), Racer->LapCount);
            }
        }
        else
        {
            WaypointIndex = Racer->WaypointsPassed;
            if (GameState && GameState->TotalWaypoints > 0 && Racer->WaypointsPassed >= GameState->TotalWaypoints)
            {
                Racer->LapCount++;
                Racer->WaypointsPassed = 0;
                WaypointIndex = 0;
                UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Completed lap %d"), *GetName(), Racer->LapCount);
            }
        }

        if (GameState)
        {
            GameState->UpdateRacerProgress(Racer, Racer->LapCount, WaypointIndex);
        }
    }

    if (bUseGraphNavigation && Graph)
    {
        TArray<AActor*> Neighbors = Graph->GetNeighbors(ReachedWaypoint);
        if (Neighbors.Num() > 0)
        {
            int32 RandomIndex = FMath::RandRange(0, Neighbors.Num() - 1);
            CurrentWaypoint = Neighbors[RandomIndex];
            int32 NextIndex = AdvancedRaceManager->Waypoints.Find(Cast<AWaypoint>(CurrentWaypoint));
            UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Randomly selected next waypoint %s (index %d) from %d neighbors"),
                *GetName(), *CurrentWaypoint->GetName(), NextIndex, Neighbors.Num());
        }
        else
        {
            // Fallback: Instead of resetting to first waypoint, loop back to a previous waypoint with neighbors
            CurrentWaypoint = AdvancedRaceManager->Waypoints[4]; // E (index 4) as a fallback, since it leads to F
            int32 FallbackIndex = 4;
            UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: No neighbors for %s (index %d), falling back to waypoint %s (index %d)"),
                *GetName(), *ReachedWaypoint->GetName(), ReachedIndex, *CurrentWaypoint->GetName(), FallbackIndex);
        }
    }
    else if (LinkedList)
    {
        CurrentWaypoint = LinkedList->GetNext(ReachedWaypoint);
        int32 NextIndex = AdvancedRaceManager->Waypoints.Find(Cast<AWaypoint>(CurrentWaypoint));
        UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Selected next waypoint %s (index %d) via LinkedList"),
            *GetName(), *CurrentWaypoint->GetName(), NextIndex);
    }

    MoveToCurrentWaypoint();
}

void AAIRacerContoller::MoveToCurrentWaypoint()
{
    if (!CurrentWaypoint)
    {
        UE_LOG(LogTemp, Error, TEXT("AIRacerContoller %s: CurrentWaypoint is null."), *GetName());
        return;
    }

    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
    {
        UE_LOG(LogTemp, Error, TEXT("AIRacerContoller %s: ControlledPawn is null."), *GetName());
        return;
    }

    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSys)
    {
        UE_LOG(LogTemp, Error, TEXT("AIRacerContoller %s: Navigation system not found!"), *GetName());
        return;
    }

    FVector RacerLocation = ControlledPawn->GetActorLocation();
    FNavLocation NavLocation;
    bool bIsOnNavMesh = NavSys->ProjectPointToNavigation(RacerLocation, NavLocation, FVector(500.0f, 500.0f, 500.0f));
    if (bIsOnNavMesh)
    {
        ControlledPawn->SetActorLocation(NavLocation.Location);
        RacerLocation = NavLocation.Location;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacerContoller %s: Racer at %s is not on NavMesh"), *GetName(), *RacerLocation.ToString());
        return;
    }

    FVector WaypointLocation = CurrentWaypoint->GetActorLocation();
    bIsOnNavMesh = NavSys->ProjectPointToNavigation(WaypointLocation, NavLocation, FVector(500.0f, 500.0f, 500.0f));
    if (bIsOnNavMesh)
    {
        WaypointLocation = NavLocation.Location;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacerContoller %s: Waypoint at %s is not on NavMesh"), *GetName(), *WaypointLocation.ToString());
        return;
    }

    int32 CurrentIndex = AdvancedRaceManager->Waypoints.Find(Cast<AWaypoint>(CurrentWaypoint));
    UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Moving to waypoint %s (index %d) at location %s"),
        *GetName(), *CurrentWaypoint->GetName(), CurrentIndex, *WaypointLocation.ToString());

    EPathFollowingRequestResult::Type Result = MoveToActor(CurrentWaypoint, 200.0f);

    switch (Result)
    {
    case EPathFollowingRequestResult::Failed:
        UE_LOG(LogTemp, Error, TEXT("AIRacerContoller %s: MoveToActor failed for waypoint %s (index %d)"),
            *GetName(), *CurrentWaypoint->GetName(), CurrentIndex);
        break;
    case EPathFollowingRequestResult::AlreadyAtGoal:
        UE_LOG(LogTemp, Warning, TEXT("AIRacerContoller %s: Already at waypoint %s (index %d)"),
            *GetName(), *CurrentWaypoint->GetName(), CurrentIndex);
        OnWaypointReached(CurrentWaypoint);
        break;
    case EPathFollowingRequestResult::RequestSuccessful:
        UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: MoveToActor successful for waypoint %s (index %d)"),
            *GetName(), *CurrentWaypoint->GetName(), CurrentIndex);
        break;
    }
}