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

    AActor* Waypoint = WaypointList->GetAt(Index);
    if (!Waypoint)
    {
        UE_LOG(LogTemp, Warning, TEXT("WaypointManager: No waypoint found at index %d."), Index);
        return nullptr;
    }

    return Waypoint;
}