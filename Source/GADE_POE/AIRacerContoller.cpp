#include "AIRacerContoller.h"
#include "Kismet/GameplayStatics.h"
#include "WaypointManager.h"
#include "CustomLinkedList.h"
#include "NavigationSystem.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"

void AAIRacerContoller::BeginPlay()
{
    Super::BeginPlay();

    WaypointManager = Cast<AWaypointManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AWaypointManager::StaticClass()));
    if (!WaypointManager || !WaypointManager->WaypointList)
    {
        UE_LOG(LogTemp, Error, TEXT("AIRacerController: Missing WaypointManager or LinkedList."));
        return;
    }

    LinkedList = WaypointManager->WaypointList;
    CurrentWaypoint = LinkedList->GetFirst();

    if (!CurrentWaypoint)
    {
        UE_LOG(LogTemp, Error, TEXT("AIRacerController: No valid first waypoint."));
        return;
    }

    MoveToCurrentWaypoint();
}

void AAIRacerContoller::OnWaypointReached(AActor* ReachedWaypoint)
{
    if (!ReachedWaypoint || ReachedWaypoint != CurrentWaypoint || !LinkedList)
        return;

    UE_LOG(LogTemp, Log, TEXT("AIRacerController: Reached waypoint %s"), *ReachedWaypoint->GetName());

    CurrentWaypoint = LinkedList->GetNext(ReachedWaypoint);
    MoveToCurrentWaypoint();
}

void AAIRacerContoller::MoveToCurrentWaypoint()
{
    if (!CurrentWaypoint)
    {
        UE_LOG(LogTemp, Error, TEXT("AIRacerController: CurrentWaypoint is null."));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("AIRacerController: Moving to waypoint %s at location %s"),
        *CurrentWaypoint->GetName(), *CurrentWaypoint->GetActorLocation().ToString());

    // Check navigation system
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSys)
    {
        UE_LOG(LogTemp, Error, TEXT("AIRacerController: Navigation system not found!"));
        return;
    }

    // Attempt to move
    EPathFollowingRequestResult::Type Result = MoveToActor(CurrentWaypoint, 200.0f); // Increased to 200.0f to match TriggerSphere
    switch (Result)
    {
    case EPathFollowingRequestResult::Failed:
        UE_LOG(LogTemp, Error, TEXT("AIRacerController: MoveToActor failed for waypoint %s"), *CurrentWaypoint->GetName());
        break;
    case EPathFollowingRequestResult::AlreadyAtGoal:
        UE_LOG(LogTemp, Warning, TEXT("AIRacerController: Already at waypoint %s"), *CurrentWaypoint->GetName());
        break;
    case EPathFollowingRequestResult::RequestSuccessful:
        UE_LOG(LogTemp, Log, TEXT("AIRacerController: MoveToActor successful for waypoint %s"), *CurrentWaypoint->GetName());
        break;
    }
}
