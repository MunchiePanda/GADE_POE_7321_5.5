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

    AAIRacer* Racer = Cast<AAIRacer>(ControlledPawn);
    if (!Racer)
    {
        return;
    }

    FVector Start = ControlledPawn->GetActorLocation();
    FVector End = CurrentWaypoint->GetActorLocation();

    // Log initial state
    UE_LOG(LogTemp, Warning, TEXT("=== AI Racer Movement Debug (%s) ==="), *GetName());
    UE_LOG(LogTemp, Warning, TEXT("Current Position: %s"), *Start.ToString());
    UE_LOG(LogTemp, Warning, TEXT("Target Waypoint: %s at %s"), *CurrentWaypoint->GetName(), *End.ToString());
    UE_LOG(LogTemp, Warning, TEXT("Direct Distance: %.2f"), FVector::Dist(Start, End));

    if (UCharacterMovementComponent* MovementComp = Racer->GetCharacterMovement())
    {
        UE_LOG(LogTemp, Warning, TEXT("Movement State:"));
        UE_LOG(LogTemp, Warning, TEXT("  Current Speed: %.2f"), MovementComp->Velocity.Size());
        UE_LOG(LogTemp, Warning, TEXT("  Max Speed: %.2f"), MovementComp->MaxWalkSpeed);
        UE_LOG(LogTemp, Warning, TEXT("  Acceleration: %.2f"), MovementComp->MaxAcceleration);
        UE_LOG(LogTemp, Warning, TEXT("  Current Velocity: %s"), *MovementComp->Velocity.ToString());
    }

    // Get the navigation system
    UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSystem)
    {
        UE_LOG(LogTemp, Error, TEXT("No Navigation System available!"));
        return;
    }

    // Log nav system state
    UE_LOG(LogTemp, Warning, TEXT("Navigation System Status:"));
    ANavigationData* NavData = NavSystem->GetDefaultNavDataInstance();
    if (NavData)
    {
        UE_LOG(LogTemp, Warning, TEXT("  Nav Data: %s"), *NavData->GetName());
        
        FNavLocation ProjectedStartLoc;
        FNavLocation ProjectedEndLoc;
        NavSystem->ProjectPointToNavigation(Start, ProjectedStartLoc, NavData->GetDefaultQueryExtent());
        NavSystem->ProjectPointToNavigation(End, ProjectedEndLoc, NavData->GetDefaultQueryExtent());
        
        FVector ProjectedStart = ProjectedStartLoc.Location;
        FVector ProjectedEnd = ProjectedEndLoc.Location;
        
        UE_LOG(LogTemp, Warning, TEXT("  Projected Start: %s"), *ProjectedStart.ToString());
        UE_LOG(LogTemp, Warning, TEXT("  Projected End: %s"), *ProjectedEnd.ToString());

        // Create path finding query with proper constructor
        FPathFindingQuery TestQuery(this, *NavData, ProjectedStart, ProjectedEnd);
        TestQuery.SetAllowPartialPaths(true);
        UE_LOG(LogTemp, Warning, TEXT("  Start on NavMesh: %s"), NavSystem->TestPathSync(TestQuery) ? TEXT("Yes") : TEXT("No"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("No Navigation Data available!"));
    }

    // Create path finding request with proper constructor
    FPathFindingQuery Query(this, *NavData, Start, End);
    Query.SetAllowPartialPaths(true);
    Query.NavData = NavData;
    
    // Set up navigation filters
    FNavigationQueryFilter Filter;
    Filter.SetMaxSearchNodes(1000);
    Query.QueryFilter = &Filter;
    
    // Find path
    FPathFindingResult Result = NavSystem->FindPathSync(Query);
    UE_LOG(LogTemp, Warning, TEXT("Path Finding Result:"));
    UE_LOG(LogTemp, Warning, TEXT("  Success: %s"), Result.IsSuccessful() ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("  Path Valid: %s"), Result.Path.IsValid() ? TEXT("Yes") : TEXT("No"));

    if (Result.IsSuccessful() && Result.Path.IsValid())
    {
        const TArray<FNavPathPoint>& PathPoints = Result.Path->GetPathPoints();
        UE_LOG(LogTemp, Warning, TEXT("  Path Points: %d"), PathPoints.Num());
        
        // Validate and adjust path points
        TArray<FNavPathPoint> AdjustedPathPoints;
        float LastValidZ = Start.Z;
        
        for (int32 i = 0; i < PathPoints.Num(); i++)
        {
            FNavPathPoint AdjustedPoint = PathPoints[i];
            
            // Project point to nav mesh
            FNavLocation ProjectedLocation;
            if (NavSystem->ProjectPointToNavigation(PathPoints[i].Location, ProjectedLocation, NavData->GetDefaultQueryExtent()))
            {
                // Keep some height above nav mesh
                AdjustedPoint.Location.Z = ProjectedLocation.Location.Z + 50.0f;
            }
            else
            {
                // If projection fails, interpolate height from last valid point
                AdjustedPoint.Location.Z = LastValidZ;
            }
            
            LastValidZ = AdjustedPoint.Location.Z;
            AdjustedPathPoints.Add(AdjustedPoint);
            
            // Draw debug visualization
            DrawDebugSphere(
                GetWorld(),
                AdjustedPoint.Location,
                25.0f,
                12,
                FColor::Yellow,
                false,
                2.0f
            );
            
            if (i < AdjustedPathPoints.Num() - 1)
            {
                DrawDebugLine(
                    GetWorld(),
                    AdjustedPoint.Location,
                    AdjustedPathPoints[i + 1].Location,
                    FColor::Green,
                    false,
                    2.0f,
                    0,
                    3.0f
                );
            }
        }
        
        // Use adjusted path for movement
        if (AAIRacer* Racer = Cast<AAIRacer>(GetPawn()))
        {
            // Calculate path characteristics
            float TotalPathLength = 0.0f;
            float MaxHeightDifference = 0.0f;
            
            for (int32 i = 0; i < AdjustedPathPoints.Num() - 1; i++)
            {
                TotalPathLength += FVector::Dist(AdjustedPathPoints[i].Location, AdjustedPathPoints[i + 1].Location);
                float HeightDiff = FMath::Abs(AdjustedPathPoints[i + 1].Location.Z - AdjustedPathPoints[i].Location.Z);
                MaxHeightDifference = FMath::Max(MaxHeightDifference, HeightDiff);
            }
            
            // Adjust movement based on path characteristics
            if (UCharacterMovementComponent* MovementComp = Racer->GetCharacterMovement())
            {
                // Reduce speed for significant height changes
                float SpeedMultiplier = 1.0f;
                if (MaxHeightDifference > 50.0f)
                {
                    SpeedMultiplier = FMath::Max(0.6f, 1.0f - (MaxHeightDifference / 200.0f));
                }
                
                MovementComp->MaxWalkSpeed = Racer->MaxSpeed * SpeedMultiplier;
                UE_LOG(LogTemp, Warning, TEXT("  Path Analysis:"));
                UE_LOG(LogTemp, Warning, TEXT("    - Total Length: %.2f"), TotalPathLength);
                UE_LOG(LogTemp, Warning, TEXT("    - Max Height Difference: %.2f"), MaxHeightDifference);
                UE_LOG(LogTemp, Warning, TEXT("    - Speed Multiplier: %.2f"), SpeedMultiplier);
            }
        }
        
        // Use UE4's built-in path following with adjusted acceptance radius
        EPathFollowingRequestResult::Type MoveResult = MoveToLocation(
            End,
            150.0f, // Increased acceptance radius
            true,
            true,
            true,
            true
        );

        UE_LOG(LogTemp, Warning, TEXT("Move Request Result: %s"), 
            MoveResult == EPathFollowingRequestResult::RequestSuccessful ? TEXT("Success") :
            MoveResult == EPathFollowingRequestResult::AlreadyAtGoal ? TEXT("Already At Goal") :
            TEXT("Failed"));

        // Draw debug path
        DrawDebugDirectionalArrow(
            GetWorld(),
            Start,
            End,
            250.0f,  // Arrow size
            FColor::Blue,
            false,   // Persistent
            2.0f,    // Duration
            0,       // DepthPriority
            5.0f     // Thickness
        );

        // Adjust speed based on path characteristics
        if (UCrowdFollowingComponent* CrowdFollowing = Cast<UCrowdFollowingComponent>(GetPathFollowingComponent()))
        {
            // Calculate path curvature for the next few points
            float MaxCurvature = 0.0f;
            const int32 LookAheadPoints = FMath::Min(5, PathPoints.Num() - 1);
            
            for (int32 i = 0; i < LookAheadPoints - 1; i++)
            {
                FVector CurrentDir = (PathPoints[i + 1].Location - PathPoints[i].Location).GetSafeNormal();
                FVector NextDir = (PathPoints[i + 2].Location - PathPoints[i + 1].Location).GetSafeNormal();
                float Curvature = FMath::Acos(FVector::DotProduct(CurrentDir, NextDir));
                MaxCurvature = FMath::Max(MaxCurvature, Curvature);
            }

            UE_LOG(LogTemp, Warning, TEXT("Path Analysis:"));
            UE_LOG(LogTemp, Warning, TEXT("  Max Curvature: %.2f degrees"), FMath::RadiansToDegrees(MaxCurvature));

            // Adjust speed based on curvature
            float SpeedMultiplier = 1.0f;
            if (MaxCurvature > PI / 6) // 30 degrees
            {
                SpeedMultiplier = FMath::Max(0.4f, 1.0f - (MaxCurvature / PI));
            }

            UE_LOG(LogTemp, Warning, TEXT("  Speed Multiplier: %.2f"), SpeedMultiplier);

            // Apply speed adjustment
            if (UCharacterMovementComponent* MovementComp = Racer->GetCharacterMovement())
            {
                MovementComp->MaxWalkSpeed = Racer->MaxSpeed * SpeedMultiplier;
                UE_LOG(LogTemp, Warning, TEXT("  Adjusted Max Speed: %.2f"), MovementComp->MaxWalkSpeed);
            }

            // Update crowd following parameters
            CrowdFollowing->SetCrowdAvoidanceQuality(ECrowdAvoidanceQuality::High);
            CrowdFollowing->SetCrowdSeparationWeight(1.2f);
            CrowdFollowing->SetCrowdCollisionQueryRange(150.0f);

            UE_LOG(LogTemp, Warning, TEXT("Crowd Following Settings:"));
            UE_LOG(LogTemp, Warning, TEXT("  Separation Range: 150.0"));
            UE_LOG(LogTemp, Warning, TEXT("  Separation Weight: 1.2"));
            UE_LOG(LogTemp, Warning, TEXT("  Avoidance Quality: High"));
        }

        // Debug visualization
        if (CVarShowDebugPath.GetValueOnGameThread())
        {
            for (int32 i = 0; i < PathPoints.Num() - 1; i++)
            {
                DrawDebugLine(GetWorld(), PathPoints[i].Location, PathPoints[i + 1].Location,
                    FColor::Blue, false, 0.1f, 0, 1.0f);
                DrawDebugSphere(GetWorld(), PathPoints[i].Location, 10.0f, 8, FColor::Red, false, 0.1f);
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Path finding failed, using fallback direct movement"));
        // Fallback behavior if path finding fails
        FVector Direction = (End - Start).GetSafeNormal();
        Racer->AddMovementInput(Direction, 1.0f);
    }

    UE_LOG(LogTemp, Warning, TEXT("=== End Movement Debug ===\n"));
}