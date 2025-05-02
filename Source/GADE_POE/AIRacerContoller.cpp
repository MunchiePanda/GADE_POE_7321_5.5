#include "AIRacerContoller.h"
#include "Kismet/GameplayStatics.h"
#include "WaypointManager.h"
#include "CustomLinkedList.h"
#include "NavigationSystem.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "AIRacer.h"
#include "BiginnerRaceGameState.h"

// Sets default values
AAIRacerContoller::AAIRacerContoller() 
{
    PrimaryActorTick.bCanEverTick = true;
}

void AAIRacerContoller::BeginPlay()
{
    Super::BeginPlay();
    // Get total waypoints from WaypointManager and LinkedList 
    WaypointManager = Cast<AWaypointManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AWaypointManager::StaticClass()));
    if (!WaypointManager || !WaypointManager->WaypointList)
    {
        UE_LOG(LogTemp, Error, TEXT("AIRacerContoller: Missing WaypointManager or LinkedList."));
        return;
    }
    // retrieve the game state
    GameState = Cast<ABeginnerRaceGameState>(GetWorld()->GetGameState());
    if (!GameState)
    {
        UE_LOG(LogTemp, Error, TEXT("AIRacerContoller: Failed to find BeginnerRaceGameState."));
        return;
    }
    // retrieve the first waypoint 
    LinkedList = WaypointManager->WaypointList;
    CurrentWaypoint = LinkedList->GetFirst();

    if (!CurrentWaypoint)
    {
        UE_LOG(LogTemp, Error, TEXT("AIRacerContoller: No valid first waypoint."));
        return;
    }

    // Defer initialization until pawn is possessed
}

void AAIRacerContoller::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Check for pawn possession on the first tick
    if (!bInitialized && GetPawn())
    {
        bInitialized = true;
        UE_LOG(LogTemp, Log, TEXT("AIRacerContoller: Pawn possessed, initializing navigation."));
        GetWorld()->GetTimerManager().SetTimer(InitialMoveTimerHandle, this, &AAIRacerContoller::DelayedMoveToCurrentWaypoint, 0.5f, false);
    }
}

void AAIRacerContoller::DelayedMoveToCurrentWaypoint()
{
    MoveToCurrentWaypoint(); // Move to the current waypoint
}
// Called when a waypoint is reached by the AI Racer 
void AAIRacerContoller::OnWaypointReached(AActor* ReachedWaypoint)
{
    if (!ReachedWaypoint || ReachedWaypoint != CurrentWaypoint || !LinkedList)
        return;

    UE_LOG(LogTemp, Log, TEXT("AIRacerContoller: Reached waypoint %s"), *ReachedWaypoint->GetName());

    // Update racer progress
    AAIRacer* Racer = Cast<AAIRacer>(GetPawn());
    if (Racer)
    {
        Racer->WaypointsPassed++;

        // Check if a lap is completed (assuming waypoints are in a loop)
        int32 TotalWaypoints = GameState->TotalWaypoints;
        if (TotalWaypoints > 0 && Racer->WaypointsPassed >= TotalWaypoints)
        {
            Racer->LapCount++;
            Racer->WaypointsPassed = 0; // Reset waypoint count after completing a lap
            UE_LOG(LogTemp, Log, TEXT("AIRacerContoller: %s completed lap %d"), *Racer->GetName(), Racer->LapCount);
        }

        // Update GameState
        if (GameState)
        {
            GameState->UpdateRacerProgress(Racer, Racer->LapCount, Racer->WaypointsPassed);
        }
    }
    // Move to the next waypoint
    CurrentWaypoint = LinkedList->GetNext(ReachedWaypoint);
    MoveToCurrentWaypoint();
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
	// Project the Racer's location to the NavMesh
    bool bIsOnNavMesh = NavSys->ProjectPointToNavigation(RacerLocation, NavLocation, FVector(500.0f, 500.0f, 500.0f));
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
        WaypointLocation = NavLocation.Location; // Set the waypoint location to the projected location on the NavMesh
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AIRacerContoller: Waypoint at %s is not on NavMesh"), *WaypointLocation.ToString());
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("AIRacerContoller: Moving to waypoint %s at location %s"),
        *CurrentWaypoint->GetName(), *WaypointLocation.ToString());

    EPathFollowingRequestResult::Type Result = MoveToActor(CurrentWaypoint, 200.0f);// Move to the current waypoint

    // Handle the result of the movement request
    switch (Result)
    {
    case EPathFollowingRequestResult::Failed: // MoveToActor failed to reach the waypoint
        UE_LOG(LogTemp, Error, TEXT("AIRacerContoller: MoveToActor failed for waypoint %s"), *CurrentWaypoint->GetName());
        break;
    case EPathFollowingRequestResult::AlreadyAtGoal: // Already at the waypoint
        UE_LOG(LogTemp, Warning, TEXT("AIRacerContoller: Already at waypoint %s"), *CurrentWaypoint->GetName());
        OnWaypointReached(CurrentWaypoint);
        break;
    case EPathFollowingRequestResult::RequestSuccessful: // MoveToActor successfully reached the waypoint
        UE_LOG(LogTemp, Log, TEXT("AIRacerContoller: MoveToActor successful for waypoint %s"), *CurrentWaypoint->GetName());
        break;
    }
}