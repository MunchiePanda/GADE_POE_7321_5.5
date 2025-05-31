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

    AAIRacer* Racer = Cast<AAIRacer>(GetPawn());
    if (Racer && CurrentWaypoint)
    {
        // Raycast to ground the racer on the track
        FVector Start = Racer->GetActorLocation();
        FVector End = Start + FVector(0, 0, -200.0f); // Cast downward 200 units
        FHitResult HitResult;
        bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility);
        if (bHit)
        {
            // Snap the racer to the track surface, keeping it slightly above to avoid jitter
            FVector NewLocation = HitResult.Location + FVector(0, 0, 10.0f); // Offset slightly above the surface
            Racer->SetActorLocation(FVector(NewLocation.X, NewLocation.Y, NewLocation.Z), false);
        }

        // Smooth movement toward the waypoint
        FVector TargetLocation = CurrentWaypoint->GetActorLocation();
        FVector CurrentLocation = Racer->GetActorLocation();
        FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
        float DistanceToTarget = FVector::Dist(CurrentLocation, TargetLocation);

        // Interpolate movement for smoother tracking
        FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetLocation, DeltaTime, 5.0f); // Adjust speed (5.0f) as needed
        Racer->SetActorLocation(FVector(NewLocation.X, NewLocation.Y, CurrentLocation.Z), false);

        // Apply movement force for physics interaction (e.g., collisions)
        FVector Force = Direction * Racer->MaxAcceleration;
        Racer->PhysicsBody->AddForce(Force);

        // Check proximity to waypoint
        if (DistanceToTarget < 200.0f)
        {
            OnWaypointReached(CurrentWaypoint);
        }
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

    int32 ReachedIndex = AdvancedRaceManager ? AdvancedRaceManager->Waypoints.Find(Cast<AWaypoint>(ReachedWaypoint)) : INDEX_NONE;
    UE_LOG(LogTemp, Log, TEXT("AIRacerController %s: Reached waypoint %s (index %d)"),
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
            UE_LOG(LogTemp, Log, TEXT("AIRacerController %s: Waypoint %s (index %d) has %d neighbors: %s (indices: %s)"),
                *GetName(), *ReachedWaypoint->GetName(), ReachedIndex, Neighbors.Num(), *NeighborNames, *NeighborIndices);

            if (WaypointIndex == 0 && Racer->WaypointsPassed > 1)
            {
                Racer->LapCount++; Racer->WaypointsPassed = 1;
                UE_LOG(LogTemp, Log, TEXT("AIRacerController %s: Completed lap %d"), *GetName(), Racer->LapCount);
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
                UE_LOG(LogTemp, Log, TEXT("AIRacerController %s: Completed lap %d"), *GetName(), Racer->LapCount);
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
            UE_LOG(LogTemp, Log, TEXT("AIRacerController %s: Randomly selected next waypoint %s (index %d) from %d neighbors"),
                *GetName(), *CurrentWaypoint->GetName(), NextIndex, Neighbors.Num());
        }
        else
        {
            CurrentWaypoint = AdvancedRaceManager->Waypoints[0];
            int32 FallbackIndex = 0;
            UE_LOG(LogTemp, Log, TEXT("AIRacerController %s: No neighbors for %s (index %d), falling back to waypoint %s (index %d)"),
                *GetName(), *ReachedWaypoint->GetName(), ReachedIndex, *CurrentWaypoint->GetName(), FallbackIndex);
        }
    }
    else if (LinkedList)
    {
        CurrentWaypoint = LinkedList->GetNext(ReachedWaypoint);
        int32 NextIndex = AdvancedRaceManager ? AdvancedRaceManager->Waypoints.Find(Cast<AWaypoint>(CurrentWaypoint)) : INDEX_NONE;
        UE_LOG(LogTemp, Log, TEXT("AIRacerController %s: Selected next waypoint %s (index %d) via LinkedList"),
            *GetName(), *CurrentWaypoint->GetName(), NextIndex);
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
}