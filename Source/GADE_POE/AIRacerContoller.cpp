/**
 Implementation of the AIRacerController class
 This file contains the core AI logic for controlling racing characters,
 including navigation, waypoint following, and path finding.
 */

#include "AIRacerContoller.h"
#include "AIRacer.h"
#include "AdvancedRaceManager.h"
#include "BiginnerRaceGameState.h"
#include "CustomLinkedList.h"
#include "Graph.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "Waypoint.h"
#include "WaypointManager.h"
#include "Kismet/GameplayStatics.h"
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
    bUseGraphNavigation = false;
    Graph = nullptr;
    AdvancedRaceManager = nullptr;
}

void AAIRacerContoller::BeginPlay()
{
    Super::BeginPlay();
    
    // Set up retry timer for initialization
    GetWorld()->GetTimerManager().SetTimer(
        InitTimerHandle,
        [this]()
        {
            // First try to find AdvancedRaceManager for graph navigation
            if (!AdvancedRaceManager)
            {
                AdvancedRaceManager = Cast<AAdvancedRaceManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AAdvancedRaceManager::StaticClass()));
            }

            // If we found AdvancedRaceManager, use graph navigation
            if (AdvancedRaceManager)
            {
                if (!Graph)
                {
                    Graph = AdvancedRaceManager->GetGraph();
                }

                if (!Graph)
                {
                    UE_LOG(LogTemp, Warning, TEXT("AIRacerContoller: Waiting for Graph initialization..."));
                    return; // Will try again next tick
                }

                // Clear the timer since we've successfully initialized
                GetWorld()->GetTimerManager().ClearTimer(InitTimerHandle);
                
                // Get the game state
                GameState = Cast<ABeginnerRaceGameState>(GetWorld()->GetGameState());
                if (!GameState)
                {
                    UE_LOG(LogTemp, Error, TEXT("AIRacerContoller: Failed to find BeginnerRaceGameState."));
                    return;
                }

                // Set up graph navigation
                bUseGraphNavigation = true;
                InitializeGraphNavigation();
                
                UE_LOG(LogTemp, Log, TEXT("AIRacerContoller: Successfully initialized with graph navigation"));
                return;
            }
            
            // If no AdvancedRaceManager found, fall back to waypoint navigation
            if (!WaypointManager)
            {
                WaypointManager = Cast<AWaypointManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AWaypointManager::StaticClass()));
            }
            
            if (!WaypointManager || !WaypointManager->WaypointList || WaypointManager->WaypointList->GetCount() == 0)
            {
                UE_LOG(LogTemp, Warning, TEXT("AIRacerContoller: Waiting for WaypointManager initialization..."));
                return; // Will try again next tick
            }
            
            // Clear the timer since we've successfully initialized
            GetWorld()->GetTimerManager().ClearTimer(InitTimerHandle);
            
            // Get the game state
            GameState = Cast<ABeginnerRaceGameState>(GetWorld()->GetGameState());
            if (!GameState)
            {
                UE_LOG(LogTemp, Error, TEXT("AIRacerContoller: Failed to find BeginnerRaceGameState."));
                return;
            }
            
            // Initialize waypoints
            LinkedList = WaypointManager->WaypointList;
            CurrentWaypoint = Cast<AWaypoint>(LinkedList->GetFirst());
            
            if (!CurrentWaypoint)
            {
                UE_LOG(LogTemp, Error, TEXT("AIRacerContoller: No valid first waypoint."));
                return;
            }
            
            // Set up waypoint navigation
            bUseGraphNavigation = false;
            InitializeWaypointNavigation();
            
            UE_LOG(LogTemp, Log, TEXT("AIRacerContoller: Successfully initialized with waypoint navigation"));
        },
        0.1f, // Check every 0.1 seconds
        true   // Loop until successful
    );
}

void AAIRacerContoller::DetermineNavigationType()
{
    if (bUseGraphNavigation && Graph)
    {
        InitializeGraphNavigation();
    }
    else
    {
        InitializeWaypointNavigation();
    }
}

void AAIRacerContoller::InitializeGraphNavigation()
{
    if (!Graph)
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacerContoller: Graph not set, falling back to waypoint navigation."));
        bUseGraphNavigation = false;
        InitializeWaypointNavigation();
        return;
    }

    // Find AdvancedRaceManager if not already set
    if (!AdvancedRaceManager)
    {
        AdvancedRaceManager = Cast<AAdvancedRaceManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AAdvancedRaceManager::StaticClass()));
    }

    if (!AdvancedRaceManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacerContoller: AdvancedRaceManager not found, falling back to waypoint navigation."));
        bUseGraphNavigation = false;
        InitializeWaypointNavigation();
        return;
    }

    // Get the first waypoint from the graph
    TArray<AActor*> AllWaypoints;
    Graph->GetAllKeys(AllWaypoints);
    
    if (AllWaypoints.Num() > 0)
    {
        CurrentWaypoint = Cast<AWaypoint>(AllWaypoints[0]);
        if (CurrentWaypoint)
        {
            bUseGraphNavigation = true;
            UE_LOG(LogTemp, Log, TEXT("AIRacerContoller: Graph navigation initialized with first waypoint: %s"), *CurrentWaypoint->GetName());
            MoveToCurrentWaypoint();
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("AIRacerContoller: Failed to cast first waypoint from graph."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AIRacerContoller: No waypoints found in graph."));
    }
}

void AAIRacerContoller::InitializeWaypointNavigation()
{
    if (!WaypointManager || !LinkedList || !CurrentWaypoint)
    {
        UE_LOG(LogTemp, Error, TEXT("AIRacerContoller: Required components for waypoint navigation missing."));
        return;
    }

    bUseGraphNavigation = false;
    UE_LOG(LogTemp, Log, TEXT("AIRacerContoller: Waypoint navigation initialized."));
}

void AAIRacerContoller::InitializeRacerPosition()
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
    {
        UE_LOG(LogTemp, Error, TEXT("AIRacerContoller: No controlled pawn to initialize position."));
        return;
    }

    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSys)
    {
        UE_LOG(LogTemp, Error, TEXT("AIRacerContoller: Navigation system not found!"));
        return;
    }

    FVector CurrentLocation = ControlledPawn->GetActorLocation();
    FNavLocation NavLocation;
    
    // Use smaller extent for more precise projection
    const FVector ProjectionExtent(100.0f, 100.0f, 200.0f);
    bool bIsOnNavMesh = NavSys->ProjectPointToNavigation(CurrentLocation, NavLocation, ProjectionExtent);
    
    if (bIsOnNavMesh)
    {
        // Keep original X and Y, but use nav mesh Z with a small offset
        FVector NewLocation = CurrentLocation;
        NewLocation.Z = NavLocation.Location.Z + 10.0f; // Small offset to ensure we're above the nav mesh
        
        ControlledPawn->SetActorLocation(NewLocation);
        
        // Log the height adjustment for debugging
        float HeightDiff = CurrentLocation.Z - NavLocation.Location.Z;
        UE_LOG(LogTemp, Log, TEXT("AIRacerContoller: Height adjusted by %f units"), HeightDiff);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacerContoller: Could not find valid NavMesh position for racer."));
    }
}

void AAIRacerContoller::InitializeGraph(AGraph* InGraph)
{
    if (!InGraph)
    {
        UE_LOG(LogTemp, Error, TEXT("AIRacerContoller: Attempted to initialize with null Graph."));
        return;
    }

    Graph = InGraph;
    bUseGraphNavigation = true;
    UE_LOG(LogTemp, Log, TEXT("AIRacerContoller: Graph navigation initialized."));
}

void AAIRacerContoller::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Check for pawn possession on the first tick
    if (!bInitialized && GetPawn())
    {
        bInitialized = true;
        UE_LOG(LogTemp, Log, TEXT("AIRacerContoller: Pawn possessed, initializing navigation."));
        InitializeRacerPosition();
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

    UE_LOG(LogTemp, Warning, TEXT("=== AI Racer Navigation Update ==="));
    UE_LOG(LogTemp, Warning, TEXT("Racer: %s"), *GetPawn()->GetName());
    UE_LOG(LogTemp, Warning, TEXT("Reached Waypoint: %s at %s"), 
        *ReachedWaypoint->GetName(), 
        *ReachedWaypoint->GetActorLocation().ToString());

    // Update racer progress
    AAIRacer* Racer = Cast<AAIRacer>(GetPawn());
    if (Racer)
    {
        Racer->WaypointsPassed++;

        // Check if a lap is completed
        int32 TotalWaypoints = GameState->TotalWaypoints;
        if (TotalWaypoints > 0 && Racer->WaypointsPassed >= TotalWaypoints)
        {
            Racer->LapCount++;
            Racer->WaypointsPassed = 0;
            UE_LOG(LogTemp, Warning, TEXT("LAP COMPLETED - Racer: %s, Lap: %d"), *Racer->GetName(), Racer->LapCount);
        }

        // Update GameState
        if (GameState)
        {
            GameState->UpdateRacerProgress(Racer, Racer->LapCount, Racer->WaypointsPassed);
        }
    }

    // Choose next waypoint based on navigation type
    if (bUseGraphNavigation && Graph)
    {
        // Get available next waypoints from the graph
        TArray<AActor*> Neighbors = Graph->GetNeighbors(CurrentWaypoint);
        
        UE_LOG(LogTemp, Warning, TEXT("Available paths from %s:"), *ReachedWaypoint->GetName());
        for (AActor* Neighbor : Neighbors)
        {
            UE_LOG(LogTemp, Warning, TEXT("  - %s"), *Neighbor->GetName());
        }
        
        if (Neighbors.Num() > 0)
        {
            // Randomly choose one of the available paths
            int32 RandomIndex = FMath::RandRange(0, Neighbors.Num() - 1);
            CurrentWaypoint = Cast<AWaypoint>(Neighbors[RandomIndex]);
            
            if (CurrentWaypoint)
            {
                UE_LOG(LogTemp, Warning, TEXT("Chosen path: %s -> %s"), 
                    *ReachedWaypoint->GetName(), 
                    *CurrentWaypoint->GetName());
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to cast chosen waypoint."));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("No neighboring waypoints found in graph for %s"), 
                *ReachedWaypoint->GetName());
        }
    }
    else if (LinkedList)
    {
        // Fallback to linked list navigation
        CurrentWaypoint = Cast<AWaypoint>(LinkedList->GetNext(ReachedWaypoint));
        if (CurrentWaypoint)
        {
            UE_LOG(LogTemp, Warning, TEXT("Using linked list navigation: %s -> %s"),
                *ReachedWaypoint->GetName(),
                *CurrentWaypoint->GetName());
        }
    }

    if (CurrentWaypoint)
    {
        UE_LOG(LogTemp, Warning, TEXT("Moving to next waypoint: %s at %s"), 
            *CurrentWaypoint->GetName(),
            *CurrentWaypoint->GetActorLocation().ToString());
        MoveToCurrentWaypoint();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to find next waypoint."));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== End Navigation Update ===\n"));
}

void AAIRacerContoller::MoveToCurrentWaypoint()
{
    if (!CurrentWaypoint)
    {
        UE_LOG(LogTemp, Error, TEXT("AIRacerContoller: CurrentWaypoint is null."));
        return;
    }

    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
    {
        UE_LOG(LogTemp, Error, TEXT("AIRacerContoller: ControlledPawn is null."));
        return;
    }

    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSys)
    {
        UE_LOG(LogTemp, Error, TEXT("AIRacerContoller: Navigation system not found!"));
        return;
    }

    // Check if the Racer is on the NavMesh
    FVector RacerLocation = ControlledPawn->GetActorLocation();
    FNavLocation NavLocation;
    bool bIsOnNavMesh = NavSys->ProjectPointToNavigation(RacerLocation, NavLocation, FVector(500.0f, 500.0f, 500.0f));
    
    // Log NavMesh status
    float NavMeshZ = NavLocation.Location.Z;
    float HeightDiff = FMath::Abs(RacerLocation.Z - NavMeshZ);
    
    UE_LOG(LogTemp, Warning, TEXT("AIRacer %s Nav Mesh Status:"), *ControlledPawn->GetName());
    UE_LOG(LogTemp, Warning, TEXT("  - On Nav Mesh: %s"), bIsOnNavMesh ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("  - Current Height: %.2f"), RacerLocation.Z);
    UE_LOG(LogTemp, Warning, TEXT("  - Nav Mesh Height: %.2f"), NavMeshZ);
    UE_LOG(LogTemp, Warning, TEXT("  - Height Difference: %.2f"), HeightDiff);
    
    if (HeightDiff > 100.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("  - WARNING: Racer may be off nav mesh or too far above/below!"));
    }

    // If the Racer is on the NavMesh, set the location to the projected location
    if (bIsOnNavMesh)
    {
        ControlledPawn->SetActorLocation(NavLocation.Location);
        RacerLocation = NavLocation.Location;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacerContoller: Racer at %s is not on NavMesh"), *RacerLocation.ToString());
        return;
    }

    // Check if the waypoint is on the NavMesh
    FVector WaypointLocation = CurrentWaypoint->GetActorLocation();
    bIsOnNavMesh = NavSys->ProjectPointToNavigation(WaypointLocation, NavLocation, FVector(500.0f, 500.0f, 500.0f));
    if (bIsOnNavMesh)
    {
        WaypointLocation = NavLocation.Location;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacerContoller: Waypoint at %s is not on NavMesh"), *WaypointLocation.ToString());
        return;
    }

    // Log distance to waypoint
    float Distance = FVector::Distance(RacerLocation, WaypointLocation);
    UE_LOG(LogTemp, Log, TEXT("AIRacerContoller: Distance to waypoint %s: %f"), 
        *CurrentWaypoint->GetName(), Distance);

    // Simple MoveToActor call - this is key for nav modifier avoidance
    EPathFollowingRequestResult::Type Result = MoveToActor(CurrentWaypoint, 200.0f);

    switch (Result)
    {
    case EPathFollowingRequestResult::Failed:
        UE_LOG(LogTemp, Error, TEXT("AIRacerContoller: MoveToActor failed for waypoint %s"), *CurrentWaypoint->GetName());
        break;
    case EPathFollowingRequestResult::AlreadyAtGoal:
        UE_LOG(LogTemp, Warning, TEXT("AIRacerContoller: Already at waypoint %s"), *CurrentWaypoint->GetName());
        OnWaypointReached(CurrentWaypoint);
        break;
    case EPathFollowingRequestResult::RequestSuccessful:
        UE_LOG(LogTemp, Log, TEXT("AIRacerContoller: MoveToActor successful for waypoint %s"), *CurrentWaypoint->GetName());
        break;
    }
}