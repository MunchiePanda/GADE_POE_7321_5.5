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

    // Find all actors with the "Waypoint" tag
    TArray<AActor*> FoundWaypoints;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Waypoint"), FoundWaypoints);

    // Sort waypoints by name to ensure consistent order (e.g., Waypoint_01, Waypoint_02, ...)
    Algo::Sort(FoundWaypoints, [](const AActor* A, const AActor* B) {
        return A->GetName() < B->GetName();
        });

    // Add sorted waypoints to the Waypoints array and linked list
    for (AActor* Waypoint : FoundWaypoints)
    {
        if (Waypoint)
        {
            Waypoints.Add(Waypoint);
            WaypointList->Add(Waypoint);
        }
    }

    if (Waypoints.Num() == 0) // No waypoints found
    {
        UE_LOG(LogTemp, Warning, TEXT("WaypointManager: No waypoints found in level!"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("WaypointManager: Found %d waypoints."), Waypoints.Num());
    }
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