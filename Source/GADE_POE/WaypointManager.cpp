#include "WaypointManager.h"
#include "Kismet/GameplayStatics.h"
#include "Algo/Sort.h"

AWaypointManager::AWaypointManager()
{
    PrimaryActorTick.bCanEverTick = false;
	WaypointList = CreateDefaultSubobject<UCustomLinkedList>(TEXT("WaypointList")); // Create the linked list
}

void AWaypointManager::BeginPlay()
{
    Super::BeginPlay();

    // Find all waypoints in the level
    TArray<AActor*> FoundWaypoints;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundWaypoints);

    TArray<AActor*> SortedWaypoints;
    for (AActor* Actor : FoundWaypoints)
    {
        // Check if the actor's name contains "BP_Waypoint_C_"
        if (Actor && Actor->GetName().Contains(TEXT("BP_Waypoint_C_")))
        {
            SortedWaypoints.Add(Actor);
        }
    }

    // Sort waypoints by name (e.g., BP_Waypoint_C_0, BP_Waypoint_C_1, etc.)
    SortedWaypoints.Sort([](const AActor& A, const AActor& B) {
        return A.GetName() < B.GetName();
        });

    // Add sorted waypoints to the linked list
    WaypointList->Clear();
    for (AActor* Waypoint : SortedWaypoints)
    {
        WaypointList->Add(Waypoint);
        UE_LOG(LogTemp, Log, TEXT("WaypointManager: Added waypoint %s to list"), *Waypoint->GetName());
    }

    UE_LOG(LogTemp, Log, TEXT("WaypointManager: Found and sorted %d waypoints."), SortedWaypoints.Num());
}


// Get a waypoint by index
AActor* AWaypointManager::GetWaypoint(int32 Index) 
{
    if (Waypoints.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("WaypointManager: No waypoints assigned! Cannot get index."));
        return nullptr;
    }

    int32 WrappedIndex = Index % Waypoints.Num(); // Loop back to start


    // Check if the wrapped index is valid
    if (Waypoints.IsValidIndex(WrappedIndex))
    {
        return Waypoints[WrappedIndex];
    }

    UE_LOG(LogTemp, Warning, TEXT("WaypointManager: Invalid wrapped index: %d"), WrappedIndex);
    return nullptr; // Return null if the index is invalid
}