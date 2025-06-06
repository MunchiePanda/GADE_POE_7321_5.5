/**
 Implementation of the AIRacerController class
 This file contains the core AI logic for controlling racing characters,
 including navigation, waypoint following, and path finding.
 */

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
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationData.h"
#include "AI/Navigation/NavigationTypes.h"
#include "Navigation/CrowdManager.h"
#include "Navigation/CrowdFollowingComponent.h"
#include "NavFilters/NavigationQueryFilter.h"

AAIRacerContoller::AAIRacerContoller()
{
    PrimaryActorTick.bCanEverTick = true;
    bInitialized = false;
    bUseGraphNavigation = true;
    CurrentWaypoint = nullptr;

    // Initialize crowd following component for advanced avoidance
    UCrowdFollowingComponent* CrowdFollowingComp = CreateDefaultSubobject<UCrowdFollowingComponent>(TEXT("CrowdFollowingComp"));
    if (CrowdFollowingComp)
    {
        CrowdFollowingComp->SuspendCrowdSteering(false);
        SetPathFollowingComponent(CrowdFollowingComp);
    }
}

void AAIRacerContoller::BeginPlay()
{
    Super::BeginPlay();

    // Configure crowd following behavior for better obstacle avoidance
    if (UCrowdFollowingComponent* CrowdFollowing = Cast<UCrowdFollowingComponent>(GetPathFollowingComponent()))
    {
        CrowdFollowing->SetCrowdSimulationState(ECrowdSimulationState::Enabled);
        CrowdFollowing->SetCrowdAvoidanceQuality(ECrowdAvoidanceQuality::High);
        CrowdFollowing->SetCrowdSeparationWeight(1.0f);
        CrowdFollowing->SetCrowdCollisionQueryRange(150.0f);
    }

    // Get reference to game state
    GameState = Cast<ABeginnerRaceGameState>(GetWorld()->GetGameState());
    if (!GameState)
    {
        UE_LOG(LogTemp, Error, TEXT("AIRacerContoller %s: Failed to find BeginnerRaceGameState."), *GetName());
        return;
    }

    // Choose between graph-based or simple waypoint navigation
    DetermineNavigationType();

    if (bUseGraphNavigation)
    {
        // Initialize graph navigation with retry timer in case dependencies aren't ready
        GetWorld()->GetTimerManager().SetTimer(InitTimerHandle, this, &AAIRacerContoller::InitializeGraphNavigation, 0.1f, true, 0.0f);
    }
    else
    {
        // Initialize simple waypoint navigation
        InitializeWaypointNavigation();
    }
}

void AAIRacerContoller::InitializeGraphNavigation()
{
    // Find and validate the race manager
    AdvancedRaceManager = Cast<AAdvancedRaceManager>(
        UGameplayStatics::GetActorOfClass(GetWorld(), AAdvancedRaceManager::StaticClass()));
    if (!AdvancedRaceManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacerContoller %s: No AdvancedRaceManager found, retrying..."), *GetName());
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Found AdvancedRaceManager."), *GetName());
    
    // Get navigation graph from race manager
    Graph = AdvancedRaceManager->GetGraph();
    if (!Graph)
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacerContoller %s: Graph is null in AdvancedRaceManager, retrying..."), *GetName());
        return;
    }

    // Stop retry timer once graph is found
    GetWorld()->GetTimerManager().ClearTimer(InitTimerHandle);
    UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Graph found."), *GetName());

    // Ensure waypoints are collected
    if (AdvancedRaceManager->Waypoints.Num() == 0)
    {
        AdvancedRaceManager->CollectWaypoints();
        UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: CollectWaypoints called, found %d waypoints."), 
            *GetName(), AdvancedRaceManager->Waypoints.Num());
    }

    // Set initial waypoint
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

    // Initialize racer's starting position
    InitializeRacerPosition();
}

void AAIRacerContoller::InitializeWaypointNavigation()
{
    // Find waypoint manager and validate waypoint list
    WaypointManager = Cast<AWaypointManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AWaypointManager::StaticClass()));
    if (!WaypointManager || !WaypointManager->WaypointList)
    {
        UE_LOG(LogTemp, Error, TEXT("AIRacerContoller %s: Missing WaypointManager or WaypointList."), *GetName());
        return;
    }

    // Get waypoint list and set initial waypoint
    LinkedList = WaypointManager->WaypointList;
    CurrentWaypoint = Cast<AWaypoint>(LinkedList->GetFirst());

    if (!CurrentWaypoint)
    {
        UE_LOG(LogTemp, Error, TEXT("AIRacerContoller %s: No valid waypoint."), *GetName());
        return;
    }

    // Initialize racer's starting position
    InitializeRacerPosition();
}

void AAIRacerContoller::InitializeRacerPosition()
{
    // Validate racer and waypoint
    AAIRacer* Racer = Cast<AAIRacer>(GetPawn());
    if (Racer && CurrentWaypoint && CurrentWaypoint->IsValidLowLevel())
    {
        UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Initialized with target waypoint %s"), 
            *GetName(), *CurrentWaypoint->GetName());
    }

    // Start navigation once pawn is possessed
    if (GetPawn())
    {
        bInitialized = true;
        UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Pawn possessed, initializing waypoints."), *GetName());
        GetWorld()->GetTimerManager().SetTimer(InitialMoveTimerHandle, this, 
            &AAIRacerContoller::DelayedMoveToCurrentWaypoint, 0.5f, false, 0.0f);
    }
}

void AAIRacerContoller::InitializeGraph(AGraph* InGraph)
{
    // Set up graph-based navigation
    Graph = InGraph;
    bUseGraphNavigation = true;

    if (Graph && AdvancedRaceManager)
    {
        // Set initial waypoint if available
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

    // Initialize navigation if not already done
    if (!bInitialized && GetPawn())
    {
        bInitialized = true;
        UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Pawn possessed, initializing navigation."), *GetName());
        GetWorld()->GetTimerManager().SetTimer(InitialMoveTimerHandle, this, 
            &AAIRacerContoller::DelayedMoveToCurrentWaypoint, 0.5f, false, 0.0f);
    }

    // Update racer movement and waypoint checking
    AAIRacer* Racer = Cast<AAIRacer>(GetPawn());
    if (Racer && CurrentWaypoint && CurrentWaypoint->IsValidLowLevel())
    {
        // Calculate direction and distance to current waypoint
        FVector WaypointLocation = CurrentWaypoint->GetActorLocation();
        FVector RacerLocation = Racer->GetActorLocation();
        FVector Direction = (WaypointLocation - RacerLocation).GetSafeNormal();
        float Distance = FVector::Dist(RacerLocation, WaypointLocation);
        UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Distance to waypoint %s: %f"), 
            *GetName(), *CurrentWaypoint->GetName(), Distance);

        // Apply movement input towards waypoint
        Racer->AddMovementInput(Direction, 1.0f);

        // Check for waypoint overlap using sphere component
        USphereComponent* WaypointSphere = Cast<USphereComponent>(
            CurrentWaypoint->GetComponentByClass(USphereComponent::StaticClass()));
        if (WaypointSphere)
        {
            // Check if racer is overlapping waypoint
            TArray<AActor*> OverlappingActors;
            WaypointSphere->GetOverlappingActors(OverlappingActors, AAIRacer::StaticClass());
            if (OverlappingActors.Contains(Racer))
            {
                UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Overlapped waypoint %s"), 
                    *GetName(), *CurrentWaypoint->GetName());
                OnWaypointReached(CurrentWaypoint);
            }
            else
            {
                UE_LOG(LogTemp, Verbose, TEXT("AIRacerContoller %s: No overlap with waypoint %s, distance: %f"), 
                    *GetName(), *CurrentWaypoint->GetName(), Distance);
            }
        }
        // Fallback to distance-based waypoint checking
        else if (Distance < 50.0f)
        {
            UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Reached waypoint %s by distance"), 
                *GetName(), *CurrentWaypoint->GetName());
            OnWaypointReached(CurrentWaypoint);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("AIRacerContoller %s: No sphere component on waypoint %s, distance: %f"), 
                *GetName(), *CurrentWaypoint->GetName(), Distance);
        }

        // Draw debug visualization
        DrawDebugLine(GetWorld(), RacerLocation, WaypointLocation, FColor::Cyan, false, 0.1f, 0, 2.f);
        DrawDebugSphere(GetWorld(), WaypointLocation, 50.f, 12, FColor::Green, false, 0.1f);
    }
    // Handle invalid waypoint
    else if (Racer && (!CurrentWaypoint || !CurrentWaypoint->IsValidLowLevel()))
    {
        // Try to recover waypoint in graph navigation mode
        if (bUseGraphNavigation && AdvancedRaceManager && AdvancedRaceManager->Waypoints.Num() > 0)
        {
            CurrentWaypoint = AdvancedRaceManager->GetWaypoint(0);
            if (CurrentWaypoint && CurrentWaypoint->IsValidLowLevel())
            {
                UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Fallback set CurrentWaypoint to %s"), 
                    *GetName(), *CurrentWaypoint->GetName());
            }
        }
        // Try to recover waypoint in simple navigation mode
        else if (!bUseGraphNavigation && LinkedList)
        {
            CurrentWaypoint = Cast<AWaypoint>(LinkedList->GetFirst());
            if (CurrentWaypoint && CurrentWaypoint->IsValidLowLevel())
            {
                UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Fallback set CurrentWaypoint to %s"), 
                    *GetName(), *CurrentWaypoint->GetName());
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

    // Get the navigation system
    UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSystem)
    {
        UE_LOG(LogTemp, Error, TEXT("No Navigation System available!"));
        return;
    }

    // Get current positions
    FVector RacerLocation = ControlledPawn->GetActorLocation();
    FVector WaypointLocation = CurrentWaypoint->GetActorLocation();

    // Project Racer to NavMesh - THIS IS CRUCIAL
    FNavLocation NavLocation;
    bool bIsOnNavMesh = NavSystem->ProjectPointToNavigation(RacerLocation, NavLocation, FVector(500.0f, 500.0f, 500.0f));
    if (bIsOnNavMesh)
    {
        ControlledPawn->SetActorLocation(NavLocation.Location);
        RacerLocation = NavLocation.Location;
        UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Projected racer to NavMesh at %s"), 
            *GetName(), *RacerLocation.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacerContoller %s: Racer at %s is not on NavMesh"), 
            *GetName(), *RacerLocation.ToString());
        return;
    }

    // Project Waypoint to NavMesh - THIS IS CRUCIAL
    bIsOnNavMesh = NavSystem->ProjectPointToNavigation(WaypointLocation, NavLocation, FVector(500.0f, 500.0f, 500.0f));
    if (bIsOnNavMesh)
    {
        WaypointLocation = NavLocation.Location;
        UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Projected waypoint to NavMesh at %s"), 
            *GetName(), *WaypointLocation.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacerContoller %s: Waypoint at %s is not on NavMesh"), 
            *GetName(), *WaypointLocation.ToString());
        return;
    }

    // Get navigation data
    ANavigationData* NavData = NavSystem->GetDefaultNavDataInstance();
    if (!NavData)
    {
        UE_LOG(LogTemp, Error, TEXT("No Navigation Data available!"));
        return;
    }

    // Create path finding query with projected locations
    FPathFindingQuery Query(this, *NavData, RacerLocation, WaypointLocation);
    Query.SetAllowPartialPaths(true);
    
    // Find path
    FPathFindingResult Result = NavSystem->FindPathSync(Query);
    
    if (Result.IsSuccessful() && Result.Path.IsValid())
    {
        // Use a larger acceptance radius for smoother movement
        EPathFollowingRequestResult::Type MoveResult = MoveToLocation(
            WaypointLocation,
            200.0f,  // Acceptance radius
            true,    // Stop on overlap
            true,    // Use path finding
            false,   // Project destination to navigation
            true     // Can strafe
        );

        switch (MoveResult)
        {
        case EPathFollowingRequestResult::Failed:
            UE_LOG(LogTemp, Warning, TEXT("AIRacerContoller %s: MoveToLocation failed for waypoint %s"), 
                *GetName(), *CurrentWaypoint->GetName());
            break;
        case EPathFollowingRequestResult::AlreadyAtGoal:
            UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Already at waypoint %s"), 
                *GetName(), *CurrentWaypoint->GetName());
            OnWaypointReached(CurrentWaypoint);
            break;
        case EPathFollowingRequestResult::RequestSuccessful:
            UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Moving to waypoint %s"), 
                *GetName(), *CurrentWaypoint->GetName());
            break;
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacerContoller %s: Failed to find path to waypoint %s"), 
            *GetName(), *CurrentWaypoint->GetName());
    }
}