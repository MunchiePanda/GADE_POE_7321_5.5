#include "AdvancedRaceManager.h"
#include "Kismet/GameplayStatics.h"
#include "Waypoint.h"
#include "Graph.h"
#include "BiginnerRaceGameState.h"

AAdvancedRaceManager::AAdvancedRaceManager()
{
    PrimaryActorTick.bCanEverTick = false;
    Graph = nullptr;
    RaceTrack = nullptr;
    GameState = nullptr;
    TotalWaypoints = 0;
    UE_LOG(LogTemp, Log, TEXT("AdvancedRaceManager %s: Constructor called."), *GetName());
}

void AAdvancedRaceManager::BeginPlay()
{
    Super::BeginPlay();

    GameState = Cast<ABeginnerRaceGameState>(GetWorld()->GetGameState());
    if (!GameState)
    {
        UE_LOG(LogTemp, Error, TEXT("AdvancedRaceManager %s: Failed to find BeginnerRaceGameState."), *GetName());
    }

    if (!Graph)
    {
        Graph = NewObject<AGraph>(this);
        UE_LOG(LogTemp, Warning, TEXT("AdvancedRaceManager %s: Graph was null, created new instance."), *GetName());
    }

    CollectWaypoints();
    PopulateGraph();

    TotalWaypoints = Waypoints.Num();
    UE_LOG(LogTemp, Log, TEXT("AdvancedRaceManager %s: Set TotalWaypoints to %d."), *GetName(), TotalWaypoints);

    UE_LOG(LogTemp, Log, TEXT("AdvancedRaceManager %s: BeginPlay completed, Graph populated with %d waypoints."), *GetName(), Waypoints.Num());
}

void AAdvancedRaceManager::InitializeTrack(AActor* RaceTrackActor, AGraph* InGraph)
{
    RaceTrack = RaceTrackActor;
    Graph = InGraph;

    if (!Graph)
    {
        Graph = NewObject<AGraph>(this);
        UE_LOG(LogTemp, Warning, TEXT("AdvancedRaceManager %s: InitializeTrack received null Graph, created new instance."), *GetName());
    }

    CollectWaypoints();
    PopulateGraph();

    TotalWaypoints = Waypoints.Num();
    UE_LOG(LogTemp, Log, TEXT("AdvancedRaceManager %s: Set TotalWaypoints to %d."), *GetName(), TotalWaypoints);

    UE_LOG(LogTemp, Log, TEXT("AdvancedRaceManager %s: Track initialized with %d waypoints."), *GetName(), Waypoints.Num());
}

void AAdvancedRaceManager::CollectWaypoints()
{
    Waypoints.Empty();
    TArray<AActor*> FoundActors;
    // Use TSubclassOf<AWaypoint> consistently
    TSubclassOf<AWaypoint> ClassToFind = WaypointClass.Get() ? WaypointClass : TSubclassOf<AWaypoint>(AWaypoint::StaticClass());
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ClassToFind, FoundActors);

    for (AActor* Actor : FoundActors)
    {
        AWaypoint* Waypoint = Cast<AWaypoint>(Actor);
        if (Waypoint && Waypoint->IsValidLowLevel())
        {
            Waypoints.Add(Waypoint);
            Graph->AddNode(Waypoint);
            UE_LOG(LogTemp, Log, TEXT("AdvancedRaceManager: Collected and added waypoint %s at %s to graph"), *Waypoint->GetName(), *Waypoint->GetActorLocation().ToString());
        }
    }
    UE_LOG(LogTemp, Log, TEXT("AdvancedRaceManager: Collected %d waypoints."), Waypoints.Num());
}

void AAdvancedRaceManager::PopulateGraph()
{
    if (!Graph || Waypoints.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("AdvancedRaceManager: Cannot populate graph, Graph is null or no waypoints."));
        return;
    }

    // Branch 1: 3->4, 3->5, 4->6, 5->6
    if (Waypoints.Num() > 6)
    {
        Graph->AddEdge(Waypoints[2], Waypoints[3]); // 3->4
        Graph->AddEdge(Waypoints[2], Waypoints[4]); // 3->5
        Graph->AddEdge(Waypoints[3], Waypoints[5]); // 4->6
        Graph->AddEdge(Waypoints[4], Waypoints[5]); // 5->6
        UE_LOG(LogTemp, Log, TEXT("AdvancedRaceManager: Added Branch 1 edges (3->4, 3->5, 4->6, 5->6)"));
    }

    // Branch 2: 8->9, 8->10, 9->11, 10->11
    if (Waypoints.Num() > 10)
    {
        Graph->AddEdge(Waypoints[7], Waypoints[8]);  // 8->9
        Graph->AddEdge(Waypoints[7], Waypoints[9]);  // 8->10
        Graph->AddEdge(Waypoints[8], Waypoints[10]); // 9->11
        Graph->AddEdge(Waypoints[9], Waypoints[10]); // 10->11
        UE_LOG(LogTemp, Log, TEXT("AdvancedRaceManager: Added Branch 2 edges (8->9, 8->10, 9->11, 10->11)"));
    }

    // Loop: 11->0
    if (Waypoints.Num() > 11)
    {
        Graph->AddEdge(Waypoints[10], Waypoints[0]); // 11->0
        UE_LOG(LogTemp, Log, TEXT("AdvancedRaceManager: Added loop edge (11->0)"));
    }

    // Main loop: 0->1, 1->2, 5->6, 6->7
    if (Waypoints.Num() > 7)
    {
        Graph->AddEdge(Waypoints[0], Waypoints[1]); // 0->1
        Graph->AddEdge(Waypoints[1], Waypoints[2]); // 1->2
        Graph->AddEdge(Waypoints[5], Waypoints[6]); // 5->6
        Graph->AddEdge(Waypoints[6], Waypoints[7]); // 6->7
        UE_LOG(LogTemp, Log, TEXT("AdvancedRaceManager: Added main loop edges (0->1, 1->2, 5->6, 6->7)"));
    }

    UE_LOG(LogTemp, Log, TEXT("AdvancedRaceManager: Graph populated with %d waypoints."), Waypoints.Num());
}

AWaypoint* AAdvancedRaceManager::GetWaypoint(int32 Index)
{
    if (Waypoints.IsValidIndex(Index))
    {
        return Waypoints[Index];
    }
    UE_LOG(LogTemp, Warning, TEXT("AdvancedRaceManager: Invalid waypoint index %d."), Index);
    return nullptr;
}