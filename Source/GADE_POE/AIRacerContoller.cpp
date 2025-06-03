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
        AdvancedRaceManager = Cast<AAdvancedRaceManager>(
            UGameplayStatics::GetActorOfClass(GetWorld(), AAdvancedRaceManager::StaticClass()));
        if (AdvancedRaceManager)
        {
            UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Found AdvancedRaceManager."), *GetName());
            Graph = AdvancedRaceManager->GetGraph();
            if (!Graph)
            {
                UE_LOG(LogTemp, Error, TEXT("AIRacerContoller %s: Graph is null in AdvancedRaceManager."), *GetName());
                return;
            }
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
                    UE_LOG(LogTemp, Error, TEXT("AIRacerContoller %s: First waypoint is invalid."), *GetName());
                }
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("AIRacerContoller %s: No waypoints in AdvancedRaceManager after collection."), *GetName());
                return;
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("AIRacerContoller %s: No AdvancedRaceManager found for graph navigation."), *GetName());
            return;
        }
    }
    else
    {
        WaypointManager = Cast<AWaypointManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AWaypointManager::StaticClass()));
        if (!WaypointManager || !WaypointManager->WaypointList)
        {
            UE_LOG(LogTemp, Error, TEXT("AIRacerContoller %s: Missing WaypointManager or WaypointList."), *GetName());
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

    AAIRacer* Racer = Cast<AAIRacer>(GetPawn());
    if (Racer && CurrentWaypoint && CurrentWaypoint->IsValidLowLevel())
    {
        FVector WaypointLocation = CurrentWaypoint->GetActorLocation();
        Racer->SetActorLocation(WaypointLocation + FVector(50.0f * GetUniqueID(), 0, 10.0f), false);
        UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Snapped to first waypoint at %s"), *GetName(), *Racer->GetActorLocation().ToString());
    }

    if (GetPawn())
    {
        bInitialized = true;
        UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Pawn possessed, initializing navigation."), *GetName());
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
                UE_LOG(LogTemp, Error, TEXT("AIRacerContoller %s: First waypoint is invalid."), *GetName());
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
        }
        else if (Distance < 100.0f)
        {
            UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Reached waypoint %s by distance"), *GetName(), *CurrentWaypoint->GetName());
            OnWaypointReached(CurrentWaypoint);
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
            CurrentWaypoint = LinkedList->GetFirst();
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
    UE_LOG(LogTemp, Verbose, TEXT("AIRacerContoller::OnWaypointReached - Processing %s: IsValidLowLevel=%s, IsInvalidOrDestroying=%s"),
        *ReachedWaypoint->GetName(),
        ReachedWaypoint->IsValidLowLevel() ? TEXT("True") : TEXT("False"),
        bIsInvalidOrDestroying ? TEXT("True") : TEXT("False"));

    if (ReachedWaypoint != CurrentWaypoint)
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
    if (bUseGraphNavigation && AdvancedRaceManager)
    {
        int32 ReachedIndex = AdvancedRaceManager->Waypoints.Find(Cast<AWaypoint>(ReachedWaypoint));
        UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Reached waypoint %s (index %d)"),
            *GetName(), *ReachedWaypoint->GetName(), ReachedIndex);

        TArray<AActor*> Neighbors = Graph->GetNeighbors(ReachedWaypoint);
        UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Found %d neighbors for %s"),
            *GetName(), Neighbors.Num(), *ReachedWaypoint->GetName());
        if (Neighbors.Num() > 0)
        {
            int32 RandomIndex = FMath::RandRange(0, Neighbors.Num() - 1);
            CurrentWaypoint = Neighbors[RandomIndex];
            if (!CurrentWaypoint || !CurrentWaypoint->IsValidLowLevel())
            {
                UE_LOG(LogTemp, Warning, TEXT("AIRacerContoller %s: Selected invalid next waypoint, falling back"), *GetName());
                CurrentWaypoint = AdvancedRaceManager->GetWaypoint(0);
            }
            int32 NextIndex = AdvancedRaceManager->Waypoints.Find(Cast<AWaypoint>(CurrentWaypoint));
            UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Selected next waypoint %s (index %d)"),
                *GetName(), *CurrentWaypoint->GetName(), NextIndex);
        }
        else
        {
            CurrentWaypoint = AdvancedRaceManager->GetWaypoint(0);
            int32 FallbackIndex = 0;
            UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: No neighbors for %s, falling back to waypoint %s (index %d)"),
                *GetName(), *ReachedWaypoint->GetName(), *CurrentWaypoint->GetName(), FallbackIndex);
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
        int32 WaypointIndex = Racer->WaypointsPassed;
        if (GameState && GameState->TotalWaypoints > 0 && Racer->WaypointsPassed >= GameState->TotalWaypoints)
        {
            Racer->LapCount++;
            Racer->WaypointsPassed = 0;
            WaypointIndex = 0;
            UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Completed lap %d"), *GetName(), Racer->LapCount);
        }

        CurrentWaypoint = LinkedList->GetNext(ReachedWaypoint);
        if (!CurrentWaypoint)
        {
            CurrentWaypoint = LinkedList->GetFirst();
            UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Looped back to first waypoint"), *GetName());
        }
        UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Moved to next waypoint %s (index %d)"),
            *GetName(), *CurrentWaypoint->GetName(), WaypointIndex);
    }

    if (GameState)
    {
        GameState->UpdateRacerProgress(Racer, Racer->LapCount, Racer->WaypointsPassed);
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

    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSys)
    {
        UE_LOG(LogTemp, Error, TEXT("AIRacerContoller %s: Navigation system not found."), *GetName());
        return;
    }

    FVector Start = ControlledPawn->GetActorLocation();
    FVector End = CurrentWaypoint->GetActorLocation();

    UNavigationPath* Path = NavSys->FindPathToLocationSynchronously(GetWorld(), Start, End, ControlledPawn);
    if (!Path || Path->IsPartial())
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacerContoller %s: Path to waypoint %s is partial or invalid."), *GetName(), *CurrentWaypoint->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Valid path to waypoint %s."), *GetName(), *CurrentWaypoint->GetName());
    }

    auto MoveResult = MoveToActor(CurrentWaypoint, 50.0f);
    switch (MoveResult)
    {
    case EPathFollowingRequestResult::Failed:
        UE_LOG(LogTemp, Error, TEXT("AIRacerContoller %s: MoveToActor FAILED for waypoint %s"), *GetName(), *CurrentWaypoint->GetName());
        break;
    case EPathFollowingRequestResult::AlreadyAtGoal:
        UE_LOG(LogTemp, Warning, TEXT("AIRacerContoller %s: Already at waypoint %s"), *GetName(), *CurrentWaypoint->GetName());
        OnWaypointReached(CurrentWaypoint);
        break;
    case EPathFollowingRequestResult::RequestSuccessful:
        UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Moving to waypoint %s at %s"), *GetName(), *CurrentWaypoint->GetName(), *End.ToString());
        break;
    }

    DrawDebugLine(GetWorld(), Start, End, FColor::Cyan, false, 5.f, 0, 2.f);
    DrawDebugSphere(GetWorld(), End, 50.f, 12, FColor::Green, false, 5.f);
}