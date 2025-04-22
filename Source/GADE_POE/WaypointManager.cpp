#include "WaypointManager.h"

AWaypointManager::AWaypointManager()
{
    PrimaryActorTick.bCanEverTick = false;

    WaypointList = CreateDefaultSubobject<UCustomLinkedList>(TEXT("WaypointList"));
}

void AWaypointManager::BeginPlay()
{
    Super::BeginPlay();

    WaypointList->Clear();
    for (AActor* Waypoint : Waypoints)
    {
        if (Waypoint)
        {
            WaypointList->Add(Waypoint);
        }
    }
}

AActor* AWaypointManager::GetWaypoint(int32 Index) const
{
    return WaypointList->GetAt(Index % WaypointList->GetCount());
}

int32 AWaypointManager::GetWaypointCount() const
{
    return WaypointList->GetCount();
}