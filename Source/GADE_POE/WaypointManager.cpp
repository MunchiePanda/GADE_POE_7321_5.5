#include "WaypointManager.h"
#include "Kismet/GameplayStatics.h"
#include "CustomLinkedList.h"

AWaypointManager::AWaypointManager()
{
    WaypointList = CreateDefaultSubobject<UCustomLinkedList>(TEXT("WaypointList"));
}

void AWaypointManager::BeginPlay()
{
    Super::BeginPlay();

    TArray<AActor*> FoundWaypoints;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundWaypoints);

    Waypoints.Empty();
    for (AActor* Actor : FoundWaypoints)
    {
        if (Actor && Actor->GetName().Contains(TEXT("BP_Waypoint_C_")))
        {
            Waypoints.Add(Actor);
        }
    }

    if (Waypoints.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("WaypointManager: No waypoints found."));
        return;
    }

    Waypoints.Sort([](const AActor& A, const AActor& B) {
        return A.GetActorLocation().X < B.GetActorLocation().X;
        });

    WaypointList->Clear();
    for (AActor* Waypoint : Waypoints)
    {
        WaypointList->Add(Waypoint);
        UE_LOG(LogTemp, Log, TEXT("WaypointManager: Added waypoint %s at %s to list"), *Waypoint->GetName(), *Waypoint->GetActorLocation().ToString());
    }

    UE_LOG(LogTemp, Log, TEXT("WaypointManager: Found and sorted %d waypoints."), Waypoints.Num());
}

AActor* AWaypointManager::GetWaypoint(int Index)
{
    if (!WaypointList)
    {
        UE_LOG(LogTemp, Warning, TEXT("WaypointManager: WaypointList is null."));
        return nullptr;
    }

    AActor* Waypoint = WaypointList->GetAt(Index); // Assumes GetAtIndex exists
    if (!Waypoint)
    {
        UE_LOG(LogTemp, Warning, TEXT("WaypointManager: No waypoint found at index %d."), Index);
        return nullptr;
    }

    return Waypoint;
}