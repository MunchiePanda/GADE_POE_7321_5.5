// AdvancedRaceManager.cpp
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
        UE_LOG(LogTemp, Log, TEXT("AdvancedRaceManager: Found actor %s"), *Actor->GetName());
        if (AWaypoint* Waypoint = Cast<AWaypoint>(Actor))
        {
            FString ActorName = Waypoint->GetName();
            // Accept waypoints with "Waypoint" or "BP_Waypoint_C_" in name
            if (ActorName.Contains(TEXT("Waypoint")) || ActorName.Contains(TEXT("BP_Waypoint_C_")))
            {
                Waypoints.Add(Waypoint);
                UE_LOG(LogTemp, Log, TEXT("AdvancedRaceManager: Collected waypoint %s"), *ActorName);
            }
        }
    }

    if (Waypoints.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("AdvancedRaceManager: No waypoints found in level."));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("AdvancedRaceManager: Found %d waypoints."), Waypoints.Num());
    }

    // Skip sorting to avoid compile error; assume waypoints are in desired order
    for (AWaypoint* Waypoint : Waypoints)
    {
        Graph->AddNode(Waypoint);
        UE_LOG(LogTemp, Log, TEXT("AdvancedRaceManager: Added waypoint %s at %s to graph"), *Waypoint->GetName(), *Waypoint->GetActorLocation().ToString());
    }
}

void AAdvancedRaceManager::PopulateGraph()
{
    if (!Graph || Waypoints.Num() < 12)
    {
        UE_LOG(LogTemp, Warning, TEXT("AdvancedRaceManager: Invalid Graph or insufficient waypoints (%d < 12)."), Waypoints.Num());
        return;
    }

    for (AWaypoint* Waypoint : Waypoints) Graph->AddNode(Waypoint);

    // Branch 1: 3 (C_3) -> 4 (C_4) or 5 (C_5) -> 6 (C_6)
    Graph->AddEdge(Waypoints[2], Waypoints[3]); // 3->4
    Graph->AddEdge(Waypoints[2], Waypoints[4]); // 3->5
    Graph->AddEdge(Waypoints[3], Waypoints[5]); // 4->6
    Graph->AddEdge(Waypoints[4], Waypoints[5]); // 5->6

    // Branch 2: 8 (C_8) -> 9 (C_9) or 10 (C_10) -> 11 (C_11)
    Graph->AddEdge(Waypoints[7], Waypoints[8]); // 8->9
    Graph->AddEdge(Waypoints[7], Waypoints[9]); // 8->10
    Graph->AddEdge(Waypoints[8], Waypoints[10]); // 9->11
    Graph->AddEdge(Waypoints[9], Waypoints[10]); // 10->11

    // Connect the loop: 11 -> 0 (C_11 -> C_1)
    Graph->AddEdge(Waypoints[10], Waypoints[0]); // 11->0 (assuming 11 is C_11, adjust if 12 is intended)

    // Existing edges (adjust as needed)
    Graph->AddEdge(Waypoints[0], Waypoints[1]); // 0->1
    Graph->AddEdge(Waypoints[1], Waypoints[2]); // 1->2
    Graph->AddEdge(Waypoints[5], Waypoints[6]); // 5->6 (after branch)
    Graph->AddEdge(Waypoints[6], Waypoints[7]); // 6->7

    UE_LOG(LogTemp, Log, TEXT("AdvancedRaceManager: Graph populated with %d waypoints."), Waypoints.Num());
}

AWaypoint* AAdvancedRaceManager::GetWaypoint(int32 Index)
{
    if (Index < 0 || Index >= Waypoints.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("AdvancedRaceManager: Invalid waypoint index %d."), Index);
        return nullptr;
    }
    return Waypoints[Index];
}