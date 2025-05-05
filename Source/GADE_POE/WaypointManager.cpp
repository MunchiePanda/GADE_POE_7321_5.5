#include "WaypointManager.h"
#include "Kismet/GameplayStatics.h"
#include "CustomLinkedList.h"

AWaypointManager::AWaypointManager() // Constructor
{
    WaypointList = CreateDefaultSubobject<UCustomLinkedList>(TEXT("WaypointList"));
}

void AWaypointManager::BeginPlay() // Called when the game starts
{
    Super::BeginPlay();

	TArray<AActor*> FoundWaypoints; // Array to hold found waypoints
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundWaypoints);

    Waypoints.Empty();
    for (AActor* Actor : FoundWaypoints)
    {
        if (Actor && Actor->GetName().Contains(TEXT("BP_Waypoint_C_")))
        {
			Waypoints.Add(Actor); // Add waypoint to the list
        }
    }

    if (Waypoints.Num() == 0) // No waypoints found
    {
        UE_LOG(LogTemp, Warning, TEXT("WaypointManager: No waypoints found."));
        return;
    }

    // Sort waypoints by the numeric part of their name
    Waypoints.Sort([](const AActor& A, const AActor& B) {
        FString NameA = A.GetName();
        FString NameB = B.GetName();

        // Extract the number after "BP_Waypoint_C_"
        int32 NumberA = 0, NumberB = 0;
        if (NameA.StartsWith(TEXT("BP_Waypoint_C_")))
        {
            FString NumberStr = NameA.Mid(14); // "BP_Waypoint_C_" is 14 characters
            NumberA = FCString::Atoi(*NumberStr);
        }
        if (NameB.StartsWith(TEXT("BP_Waypoint_C_")))
        {
            FString NumberStr = NameB.Mid(14);
            NumberB = FCString::Atoi(*NumberStr);
        }

        return NumberA < NumberB;
        });

    WaypointList->Clear(); // Clear the list
    for (AActor* Waypoint : Waypoints)
    {
        WaypointList->Add(Waypoint);
        UE_LOG(LogTemp, Log, TEXT("WaypointManager: Added waypoint %s at %s to list"), *Waypoint->GetName(), *Waypoint->GetActorLocation().ToString());
    }

    UE_LOG(LogTemp, Log, TEXT("WaypointManager: Found and sorted %d waypoints."), Waypoints.Num());
}

AActor* AWaypointManager::GetWaypoint(int Index) // Get a waypoint by index
{
    if (!WaypointList) // Check if the list is valid
    {
        UE_LOG(LogTemp, Warning, TEXT("WaypointManager: WaypointList is null."));
        return nullptr;
    }

    AActor* Waypoint = WaypointList->GetAt(Index); // Get the waypoint
    if (!Waypoint)
    {
        UE_LOG(LogTemp, Warning, TEXT("WaypointManager: No waypoint found at index %d."), Index);
        return nullptr;
    }

	return Waypoint; // Return the waypoint
}