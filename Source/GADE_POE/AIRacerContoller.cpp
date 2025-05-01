#include "AIRacerContoller.h"
#include "Kismet/GameplayStatics.h"
#include "WaypointManager.h"
#include "CustomLinkedList.h"
#include "NavigationSystem.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"

AAIRacerContoller::AAIRacerContoller()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AAIRacerContoller::BeginPlay()
{
    Super::BeginPlay();

    WaypointManager = Cast<AWaypointManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AWaypointManager::StaticClass()));
    if (!WaypointManager || !WaypointManager->WaypointList)
    {
        UE_LOG(LogTemp, Error, TEXT("AIRacerContoller: Missing WaypointManager or LinkedList."));
        return;
    }

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
    MoveToCurrentWaypoint();
}

void AAIRacerContoller::OnWaypointReached(AActor* ReachedWaypoint)
{
    if (!ReachedWaypoint || ReachedWaypoint != CurrentWaypoint || !LinkedList)
        return;

    UE_LOG(LogTemp, Log, TEXT("AIRacerContoller: Reached waypoint %s"), *ReachedWaypoint->GetName());

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
        UE_LOG(LogTemp, Warning, TEXT("AIRacerContoller: Racer at %s is not on NavMesh"), *RacerLocation.ToString());
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
        UE_LOG(LogTemp, Warning, TEXT("AIRacerContoller: Waypoint at %s is not on NavMesh"), *WaypointLocation.ToString());
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("AIRacerContoller: Moving to waypoint %s at location %s"),
        *CurrentWaypoint->GetName(), *WaypointLocation.ToString());

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