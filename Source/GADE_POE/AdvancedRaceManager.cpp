#include "AdvancedRaceManager.h"
#include "Kismet/GameplayStatics.h"
#include "BiginnerRaceGameState.h"
#include "Waypoint.h"
#include "Graph.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

AAdvancedRaceManager::AAdvancedRaceManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AAdvancedRaceManager::BeginPlay()
{
    Super::BeginPlay();

    GameState = Cast<ABeginnerRaceGameState>(GetWorld()->GetGameState());
    if (!GameState)
    {
        UE_LOG(LogTemp, Error, TEXT("AdvancedRaceManager: Failed to find BeginnerRaceGameState."));
    }

    // Initialize Graph if not set
    if (!Graph)
    {
        Graph = Cast<AGraph>(UGameplayStatics::GetActorOfClass(GetWorld(), AGraph::StaticClass()));
        if (!Graph)
        {
            UE_LOG(LogTemp, Warning, TEXT("AdvancedRaceManager: No Graph actor found. Creating new Graph."));
            Graph = GetWorld()->SpawnActor<AGraph>();
        }
    }

    CollectWaypoints();
    PopulateGraph();
}

void AAdvancedRaceManager::InitializeTrack(AActor* RaceTrackActor, AGraph* InGraph)
{
    RaceTrack = RaceTrackActor;
    Graph = InGraph;
}

void AAdvancedRaceManager::CollectWaypoints()
{
    if (!Graph)
    {
        UE_LOG(LogTemp, Error, TEXT("AdvancedRaceManager: Graph is null."));
        return;
    }

    Waypoints.Empty();
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWaypoint::StaticClass(), FoundActors);
    for (AActor* Actor : FoundActors)
    {
        if (!Actor || !Actor->IsValidLowLevel())
        {
            UE_LOG(LogTemp, Warning, TEXT("AdvancedRaceManager: Skipped invalid actor"));
            continue;
        }
        UE_LOG(LogTemp, Log, TEXT("AdvancedRaceManager: Found actor %s"), *Actor->GetName());
        if (AWaypoint* Waypoint = Cast<AWaypoint>(Actor))
        {
            FString ActorName = Waypoint->GetName();
            // Accept waypoints with "Waypoint" or "BP_Waypoint_C_" in name
            if (ActorName.Contains(TEXT("Waypoint")) || ActorName.Contains(TEXT("BP_Waypoint_C_")))
            {
                Waypoints.Add(Waypoint);
                Graph->AddNode(Waypoint);
                UE_LOG(LogTemp, Log, TEXT("AdvancedRaceManager: Collected and added waypoint %s at %s to graph"),
                    *ActorName, *Waypoint->GetActorLocation().ToString());
            }
        }
    }

    if (Waypoints.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("AdvancedRaceManager: No waypoints found in level."));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("AdvancedRaceManager: Collected %d waypoints."), Waypoints.Num());
    }
}

void AAdvancedRaceManager::PopulateGraph()
{
    if (!Graph)
    {
        UE_LOG(LogTemp, Error, TEXT("AdvancedRaceManager: Graph is null."));
        return;
    }

    if (Waypoints.Num() < 12)
    {
        UE_LOG(LogTemp, Warning, TEXT("AdvancedRaceManager: Insufficient waypoints (%d < 12)."), Waypoints.Num());
        return;
    }

    // Validate waypoints
    for (int32 i = 0; i < Waypoints.Num(); i++)
    {
        if (!Waypoints[i] || !Waypoints[i]->IsValidLowLevel())
        {
            UE_LOG(LogTemp, Error, TEXT("AdvancedRaceManager: Invalid waypoint at index %d"), i);
            return;
        }
    }

    // Branch 1: 3 (C_3) -> 4 (C_4) or 5 (C_5) -> 6 (C_6)
    Graph->AddEdge(Waypoints[2], Waypoints[3]); // 3->4
    Graph->AddEdge(Waypoints[2], Waypoints[4]); // 3->5
    Graph->AddEdge(Waypoints[3], Waypoints[5]); // 4->6
    Graph->AddEdge(Waypoints[4], Waypoints[5]); // 5->6
    UE_LOG(LogTemp, Log, TEXT("AdvancedRaceManager: Added Branch 1 edges (3->4, 3->5, 4->6, 5->6)"));

    // Branch 2: 8 (C_8) -> 9 (C_9) or 10 (C_10) -> 11 (C_11)
    Graph->AddEdge(Waypoints[7], Waypoints[8]); // 8->9
    Graph->AddEdge(Waypoints[7], Waypoints[9]); // 8->10
    Graph->AddEdge(Waypoints[8], Waypoints[10]); // 9->11
    Graph->AddEdge(Waypoints[9], Waypoints[10]); // 10->11
    UE_LOG(LogTemp, Log, TEXT("AdvancedRaceManager: Added Branch 2 edges (8->9, 8->10, 9->11, 10->11)"));

    // Connect the loop: 11 -> 0 (C_11 -> C_1)
    Graph->AddEdge(Waypoints[10], Waypoints[0]); // 11->0
    UE_LOG(LogTemp, Log, TEXT("AdvancedRaceManager: Added loop edge (11->0)"));

    // Existing edges
    Graph->AddEdge(Waypoints[0], Waypoints[1]); // 0->1
    Graph->AddEdge(Waypoints[1], Waypoints[2]); // 1->2
    Graph->AddEdge(Waypoints[5], Waypoints[6]); // 5->6
    Graph->AddEdge(Waypoints[6], Waypoints[7]); // 6->7
    UE_LOG(LogTemp, Log, TEXT("AdvancedRaceManager: Added main loop edges (0->1, 1->2, 5->6, 6->7)"));

    UE_LOG(LogTemp, Log, TEXT("AdvancedRaceManager: Graph populated with %d waypoints."), Waypoints.Num());
}

AWaypoint* AAdvancedRaceManager::GetWaypoint(int32 Index)
{
    if (Index < 0 || Index >= Waypoints.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("AdvancedRaceManager: Invalid waypoint index %d."), Index);
        return nullptr;
    }
    if (!Waypoints[Index] || !Waypoints[Index]->IsValidLowLevel())
    {
        UE_LOG(LogTemp, Warning, TEXT("AdvancedRaceManager: Waypoint at index %d is invalid."), Index);
        return nullptr;
    }
    return Waypoints[Index];
}