#include "AIRacerContoller.h"
#include "Kismet/GameplayStatics.h"
#include "WaypointManager.h"
#include "CustomLinkedList.h"
#include "Graph.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "AIRacer.h"
#include "BiginnerRaceGameState.h"
#include "AdvancedRaceManager.h"
#include "DrawDebugHelpers.h"
#include "Components/SphereComponent.h"

AAIRacerContoller::AAIRacerContoller()
{
    PrimaryActorTick.bCanEverTick = true;
    bInitialized = false;
    bUseGraphNavigation = true;
    CurrentWaypoint = nullptr;
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

    DetermineNavigationType();

    if (bUseGraphNavigation)
    {
        GetWorld()->GetTimerManager().SetTimer(InitTimerHandle, this, &AAIRacerContoller::InitializeGraphNavigation, 0.1f, true, 0.0f);
    }
    else
    {
        InitializeWaypointNavigation();
    }
}

void AAIRacerContoller::InitializeGraphNavigation()
{
    AdvancedRaceManager = Cast<AAdvancedRaceManager>(
        UGameplayStatics::GetActorOfClass(GetWorld(), AAdvancedRaceManager::StaticClass()));
    if (!AdvancedRaceManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacerContoller %s: No AdvancedRaceManager found, retrying..."), *GetName());
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Found AdvancedRaceManager."), *GetName());
    Graph = AdvancedRaceManager->GetGraph();
    if (!Graph)
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacerContoller %s: Graph is null in AdvancedRaceManager, retrying..."), *GetName());
        return;
    }

    GetWorld()->GetTimerManager().ClearTimer(InitTimerHandle);
    UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Graph found."), *GetName());

    if (AdvancedRaceManager->Waypoints.Num() == 0)
    {
        AdvancedRaceManager->CollectWaypoints();
        UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: CollectWaypoints called, found %d waypoints."), *GetName(), AdvancedRaceManager->Waypoints.Num());
    }

    if (AdvancedRaceManager->Waypoints.Num() > 0)
    {
        CurrentWaypoint = AdvancedRaceManager->GetWaypoint(0);
        if (CurrentWaypoint)
        {
            UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Initialized CurrentWaypoint to %s (index 0)"),
                *GetName(), *CurrentWaypoint->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("AIRacerContoller %s: First waypoint is null."), *GetName());
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AIRacerContoller %s: No waypoints in AdvancedRaceManager after collection."), *GetName());
        return;
    }

    InitializeRacerPosition();
}

void AAIRacerContoller::InitializeWaypointNavigation()
{
    WaypointManager = Cast<AWaypointManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AWaypointManager::StaticClass()));
    if (!WaypointManager || !WaypointManager->WaypointList)
    {
        UE_LOG(LogTemp, Error, TEXT("AIRacerContoller %s: Missing WaypointManager or WaypointList."), *GetName());
        return;
    }

    LinkedList = WaypointManager->WaypointList;
    CurrentWaypoint = Cast<AWaypoint>(LinkedList->GetFirst());

    if (!CurrentWaypoint)
    {
        UE_LOG(LogTemp, Error, TEXT("AIRacerContoller %s: No valid waypoint."), *GetName());
        return;
    }

    InitializeRacerPosition();
}

void AAIRacerContoller::InitializeRacerPosition()
{
    AAIRacer* Racer = Cast<AAIRacer>(GetPawn());
    if (Racer && CurrentWaypoint && CurrentWaypoint->IsValidLowLevel())
    {
        // Don't move the racer, just initialize the waypoint target
        UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Initialized with target waypoint %s"), 
            *GetName(), *CurrentWaypoint->GetName());
    }

    if (GetPawn())
    {
        bInitialized = true;
        UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Pawn possessed, initializing waypoints."), *GetName());
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
            CurrentWaypoint = AdvancedRaceManager->GetWaypoint(0);
            if (CurrentWaypoint)
            {
                bInitialized = false;
                UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Initialized Graph with CurrentWaypoint %s (index 0)"),
                    *GetName(), *CurrentWaypoint->GetName());
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("AIRacerContoller %s: First waypoint is null."), *GetName());
            }
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

    AAIRacer* Racer = Cast<AAIRacer>(GetPawn());
    if (Racer && CurrentWaypoint && CurrentWaypoint->IsValidLowLevel())
    {
        FVector WaypointLocation = CurrentWaypoint->GetActorLocation();
        FVector RacerLocation = Racer->GetActorLocation();
        FVector Direction = (WaypointLocation - RacerLocation).GetSafeNormal();
        float Distance = FVector::Dist(RacerLocation, WaypointLocation);
        UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Distance to waypoint %s: %f"), *GetName(), *CurrentWaypoint->GetName(), Distance);

        Racer->AddMovementInput(Direction, 1.0f);

        USphereComponent* WaypointSphere = Cast<USphereComponent>(CurrentWaypoint->GetComponentByClass(USphereComponent::StaticClass()));
        if (WaypointSphere)
        {
            TArray<AActor*> OverlappingActors;
            WaypointSphere->GetOverlappingActors(OverlappingActors, AAIRacer::StaticClass());
            if (OverlappingActors.Contains(Racer))
            {
                UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Overlapped waypoint %s"), *GetName(), *CurrentWaypoint->GetName());
                OnWaypointReached(CurrentWaypoint);
            }
            else
            {
                UE_LOG(LogTemp, Verbose, TEXT("AIRacerContoller %s: No overlap with waypoint %s, distance: %f"), *GetName(), *CurrentWaypoint->GetName(), Distance);
            }
        }
        else if (Distance < 50.0f)
        {
            UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Reached waypoint %s by distance"), *GetName(), *CurrentWaypoint->GetName());
            OnWaypointReached(CurrentWaypoint);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("AIRacerContoller %s: No sphere component on waypoint %s, distance: %f"), *GetName(), *CurrentWaypoint->GetName(), Distance);
        }

        DrawDebugLine(GetWorld(), RacerLocation, WaypointLocation, FColor::Cyan, false, 0.1f, 0, 2.f);
        DrawDebugSphere(GetWorld(), WaypointLocation, 50.f, 12, FColor::Green, false, 0.1f);
    }
    else if (Racer && (!CurrentWaypoint || !CurrentWaypoint->IsValidLowLevel()))
    {
        if (bUseGraphNavigation && AdvancedRaceManager && AdvancedRaceManager->Waypoints.Num() > 0)
        {
            CurrentWaypoint = AdvancedRaceManager->GetWaypoint(0);
            if (CurrentWaypoint && CurrentWaypoint->IsValidLowLevel())
            {
                UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Fallback set CurrentWaypoint to %s"), *GetName(), *CurrentWaypoint->GetName());
            }
        }
        else if (!bUseGraphNavigation && LinkedList)
        {
            CurrentWaypoint = Cast<AWaypoint>(LinkedList->GetFirst());
            if (CurrentWaypoint && CurrentWaypoint->IsValidLowLevel())
            {
                UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Fallback set CurrentWaypoint to %s"), *GetName(), *CurrentWaypoint->GetName());
            }
        }
    }
}

void AAIRacerContoller::DelayedMoveToCurrentWaypoint()
{
    MoveToCurrentWaypoint();
}

void AAIRacerContoller::OnWaypointReached(AActor* ReachedWaypoint)
{
    if (!ReachedWaypoint || !ReachedWaypoint->IsValidLowLevel())
    {
        UE_LOG(LogTemp, Error, TEXT("AIRacerContoller %s: ReachedWaypoint is invalid or null"), *GetName());
        return;
    }

    bool bIsInvalidOrDestroying = !IsValid(ReachedWaypoint) || !ReachedWaypoint->HasActorBegunPlay() || ReachedWaypoint->IsActorBeingDestroyed();
    UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Processing waypoint %s"), *GetName(), *ReachedWaypoint->GetName());
    UE_LOG(LogTemp, Log, TEXT("  - IsValidLowLevel: %s"), ReachedWaypoint->IsValidLowLevel() ? TEXT("True") : TEXT("False"));
    UE_LOG(LogTemp, Log, TEXT("  - IsInvalidOrDestroying: %s"), bIsInvalidOrDestroying ? TEXT("True") : TEXT("False"));

    AWaypoint* ReachedWaypointCast = Cast<AWaypoint>(ReachedWaypoint);
    if (ReachedWaypointCast != CurrentWaypoint)
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacerContoller %s: ReachedWaypoint %s does not match CurrentWaypoint %s"),
            *GetName(), *ReachedWaypoint->GetName(), CurrentWaypoint ? *CurrentWaypoint->GetName() : TEXT("null"));
        return;
    }

    AAIRacer* Racer = Cast<AAIRacer>(GetPawn());
    if (!Racer)
    {
        UE_LOG(LogTemp, Error, TEXT("AIRacerContoller %s: No valid pawn"), *GetName());
        return;
    }

    Racer->WaypointsPassed++;
    UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Waypoints passed: %d"), *GetName(), Racer->WaypointsPassed);

    if (bUseGraphNavigation && AdvancedRaceManager)
    {
        int32 ReachedIndex = AdvancedRaceManager->Waypoints.Find(ReachedWaypointCast);
        UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Using graph navigation, current waypoint index: %d"), *GetName(), ReachedIndex);

        TArray<AActor*> Neighbors = Graph->GetNeighbors(ReachedWaypoint);
        UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Found %d neighbors for waypoint %s"), 
            *GetName(), Neighbors.Num(), *ReachedWaypoint->GetName());

        if (Neighbors.Num() > 0)
        {
            int32 RandomIndex = FMath::RandRange(0, Neighbors.Num() - 1);
            CurrentWaypoint = Cast<AWaypoint>(Neighbors[RandomIndex]);
            if (!CurrentWaypoint || !CurrentWaypoint->IsValidLowLevel())
            {
                UE_LOG(LogTemp, Warning, TEXT("AIRacerContoller %s: Selected invalid next waypoint, falling back"), *GetName());
                CurrentWaypoint = AdvancedRaceManager->GetWaypoint(0);
            }
            int32 NextIndex = AdvancedRaceManager->Waypoints.Find(CurrentWaypoint);
            UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Selected next waypoint %s (index %d)"),
                *GetName(), *CurrentWaypoint->GetName(), NextIndex);
        }
        else
        {
            CurrentWaypoint = AdvancedRaceManager->GetWaypoint(0);
            UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: No neighbors found, falling back to first waypoint"), *GetName());
        }

        if (ReachedIndex == 0 && Racer->WaypointsPassed > 1)
        {
            Racer->LapCount++;
            Racer->WaypointsPassed = 1;
            UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Completed lap %d"), *GetName(), Racer->LapCount);
        }
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Using waypoint list navigation"), *GetName());
        CurrentWaypoint = Cast<AWaypoint>(LinkedList->GetNext(ReachedWaypoint));
        if (!CurrentWaypoint)
        {
            CurrentWaypoint = Cast<AWaypoint>(LinkedList->GetFirst());
            UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Reached end of waypoint list, looping back to start"), *GetName());
        }
        UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Next waypoint set to %s"), 
            *GetName(), *CurrentWaypoint->GetName());
    }

    if (GameState)
    {
        GameState->UpdateRacerProgress(Racer, Racer->LapCount, Racer->WaypointsPassed);
        UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Updated race progress - Lap: %d, Waypoints: %d"), 
            *GetName(), Racer->LapCount, Racer->WaypointsPassed);
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
        UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Using WaypointManager for BeginnerMap."), *GetName());
    }
    else if (CurrentLevelName == "AdvancedMap")
    {
        bUseGraphNavigation = true;
        UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Using AdvancedRaceManager for AdvancedMap."), *GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacerContoller %s: Unknown map '%s', defaulting to graph navigation."), *GetName(), *CurrentLevelName);
    }
}

void AAIRacerContoller::MoveToCurrentWaypoint()
{
    if (!CurrentWaypoint || !CurrentWaypoint->IsValidLowLevel())
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacerContoller %s: CurrentWaypoint is null or invalid."), *GetName());
        return;
    }

    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacerContoller %s: No pawn to control."), *GetName());
        return;
    }

    FVector Start = ControlledPawn->GetActorLocation();
    FVector End = CurrentWaypoint->GetActorLocation();
    float DistanceToWaypoint = FVector::Dist(Start, End);

    UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Moving to waypoint %s"), *GetName(), *CurrentWaypoint->GetName());
    UE_LOG(LogTemp, Log, TEXT("  - Distance to waypoint: %.2f"), DistanceToWaypoint);
    UE_LOG(LogTemp, Log, TEXT("  - Start Location: %s"), *Start.ToString());
    UE_LOG(LogTemp, Log, TEXT("  - End Location: %s"), *End.ToString());

    // Use direct movement with acceptance radius
    auto MoveResult = MoveToLocation(End, 200.0f, true, true, false, true);
    switch (MoveResult)
    {
    case EPathFollowingRequestResult::Failed:
        UE_LOG(LogTemp, Warning, TEXT("AIRacerContoller %s: MoveToLocation FAILED for waypoint %s"), *GetName(), *CurrentWaypoint->GetName());
        UE_LOG(LogTemp, Warning, TEXT("  - Falling back to direct movement"));
        // If movement fails, use direct input
        if (AAIRacer* Racer = Cast<AAIRacer>(ControlledPawn))
        {
            FVector Direction = (End - Start).GetSafeNormal();
            Racer->AddMovementInput(Direction, 1.0f);
            UE_LOG(LogTemp, Log, TEXT("  - Direct movement direction: %s"), *Direction.ToString());
        }
        break;
    case EPathFollowingRequestResult::AlreadyAtGoal:
        UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Already at waypoint %s"), *GetName(), *CurrentWaypoint->GetName());
        OnWaypointReached(CurrentWaypoint);
        break;
    case EPathFollowingRequestResult::RequestSuccessful:
        UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Successfully started moving to waypoint %s"), *GetName(), *CurrentWaypoint->GetName());
        break;
    }

    DrawDebugLine(GetWorld(), Start, End, FColor::Cyan, false, 5.f, 0, 2.f);
}