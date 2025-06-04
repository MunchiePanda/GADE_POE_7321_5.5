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
#include "NavFilters/NavigationQueryFilter.h"

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
        float Distance = FVector::Dist(RacerLocation, WaypointLocation);

        // Calculate steering direction with obstacle avoidance
        FVector DesiredDirection = (WaypointLocation - RacerLocation).GetSafeNormal();
        FVector SteeringDirection = CalculateSteeringDirection(RacerLocation, DesiredDirection);
        
        // Apply movement
        Racer->AddMovementInput(SteeringDirection, 1.0f);

        // Check for waypoint overlap
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
        else if (Distance < 200.0f)
        {
            UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Reached waypoint %s by distance"), *GetName(), *CurrentWaypoint->GetName());
            OnWaypointReached(CurrentWaypoint);
        }

        // Draw debug visualization
        DrawDebugLine(GetWorld(), RacerLocation, RacerLocation + SteeringDirection * 200.0f, FColor::Yellow, false, 0.1f, 0, 2.f);
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
    UE_LOG(LogTemp, Verbose, TEXT("AIRacerContoller::OnWaypointReached - Processing %s: IsValidLowLevel=%s, IsInvalidOrDestroying=%s"),
        *ReachedWaypoint->GetName(),
        ReachedWaypoint->IsValidLowLevel() ? TEXT("True") : TEXT("False"),
        bIsInvalidOrDestroying ? TEXT("True") : TEXT("False"));

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
    if (bUseGraphNavigation && AdvancedRaceManager)
    {
        int32 ReachedIndex = AdvancedRaceManager->Waypoints.Find(ReachedWaypointCast);
        UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Reached waypoint %s (index %d)"),
            *GetName(), *ReachedWaypoint->GetName(), ReachedIndex);

        TArray<AActor*> Neighbors = Graph->GetNeighbors(ReachedWaypoint);
        UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Found %d neighbors for %s"),
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

        CurrentWaypoint = Cast<AWaypoint>(LinkedList->GetNext(ReachedWaypoint));
        if (!CurrentWaypoint)
        {
            CurrentWaypoint = Cast<AWaypoint>(LinkedList->GetFirst());
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
    // This function is now mainly used for initialization
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

    // Movement is now handled in Tick function
    UE_LOG(LogTemp, Log, TEXT("AIRacerContoller %s: Initialized movement towards waypoint %s"), 
        *GetName(), *CurrentWaypoint->GetName());
}

FVector AAIRacerContoller::CalculateSteeringDirection(const FVector& RacerLocation, const FVector& DesiredDirection)
{
    // Parameters for obstacle detection
    const float DetectionRange = 500.0f;
    const float DetectionAngle = 90.0f;
    const int32 NumRays = 8;
    const float AvoidanceWeight = 0.7f;
    const float TargetWeight = 0.3f;

    FVector SteeringDirection = DesiredDirection;
    FVector AvoidanceDirection = FVector::ZeroVector;
    int32 NumObstaclesDetected = 0;

    // Cast rays in an arc to detect obstacles
    for (int32 i = 0; i < NumRays; ++i)
    {
        float Angle = FMath::DegreesToRadians(DetectionAngle * ((float)i / (NumRays - 1) - 0.5f));
        FVector RayDirection = DesiredDirection.RotateAngleAxis(Angle, FVector(0, 0, 1));
        
        FHitResult HitResult;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(GetPawn());
        
        // Line trace to detect obstacles
        if (GetWorld()->LineTraceSingleByChannel(
            HitResult,
            RacerLocation,
            RacerLocation + RayDirection * DetectionRange,
            ECC_Visibility,
            QueryParams))
        {
            // Calculate avoidance vector
            FVector ObstacleDirection = (HitResult.Location - RacerLocation).GetSafeNormal();
            float Distance = FVector::Dist(RacerLocation, HitResult.Location);
            float AvoidanceStrength = 1.0f - FMath::Clamp(Distance / DetectionRange, 0.0f, 1.0f);
            
            // Add weighted avoidance direction
            AvoidanceDirection -= ObstacleDirection * AvoidanceStrength;
            NumObstaclesDetected++;

            // Draw debug ray
            DrawDebugLine(GetWorld(), RacerLocation, HitResult.Location, FColor::Red, false, 0.1f, 0, 2.0f);
        }
        else
        {
            // Draw debug ray for no hit
            DrawDebugLine(GetWorld(), RacerLocation, RacerLocation + RayDirection * DetectionRange, FColor::Green, false, 0.1f, 0, 1.0f);
        }
    }

    // If obstacles were detected, combine avoidance and desired direction
    if (NumObstaclesDetected > 0)
    {
        AvoidanceDirection.Normalize();
        SteeringDirection = (AvoidanceDirection * AvoidanceWeight + DesiredDirection * TargetWeight).GetSafeNormal();
    }

    return SteeringDirection;
}