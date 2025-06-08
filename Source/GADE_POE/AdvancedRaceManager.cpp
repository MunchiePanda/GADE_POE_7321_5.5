#include "AdvancedRaceManager.h"
#include "Kismet/GameplayStatics.h"
#include "Waypoint.h"
#include "Graph.h"
#include "BiginnerRaceGameState.h"

// Sets default values 
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
    if (!GameState) // Check if the game state is valid
    {
        UE_LOG(LogTemp, Error, TEXT("AdvancedRaceManager %s: Failed to find BeginnerRaceGameState."), *GetName());
    }

    if (!Graph)
    {
		Graph = NewObject<AGraph>(this); // Create a new instance of AGraph if it is null
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
 
void AAdvancedRaceManager::CollectWaypoints() // Collect waypoints from the world and add them to the graph 
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

    // Update TotalWaypoints
    TotalWaypoints = Waypoints.Num();
    
    // Update GameState's TotalWaypoints
    if (!GameState)
    {
        GameState = Cast<ABeginnerRaceGameState>(GetWorld()->GetGameState());
    }
    
    if (GameState)
    {
        GameState->TotalWaypoints = TotalWaypoints;
        UE_LOG(LogTemp, Warning, TEXT("AdvancedRaceManager: Updated GameState TotalWaypoints to %d"), TotalWaypoints);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AdvancedRaceManager: GameState is null, cannot update TotalWaypoints"));
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
     
    UE_LOG(LogTemp, Warning, TEXT("=== Advanced Race Track Waypoint Order ==="));
    UE_LOG(LogTemp, Warning, TEXT("Track Layout:"));
    UE_LOG(LogTemp, Warning, TEXT("Main Path: 0->1->2->3"));
    UE_LOG(LogTemp, Warning, TEXT("Branch 1: 3->[4 or 5]->6->7->8"));
    UE_LOG(LogTemp, Warning, TEXT("Branch 2: 8->[9 or 10]->11->0 (loop)"));
    UE_LOG(LogTemp, Warning, TEXT(""));
    UE_LOG(LogTemp, Warning, TEXT("Waypoint Positions:"));
    
    // Log all waypoint positions
    for (int32 i = 0; i < Waypoints.Num(); i++)
    {
        if (Waypoints[i])
        {
            UE_LOG(LogTemp, Warning, TEXT("Waypoint %d: %s at location %s"), 
                i, 
                *Waypoints[i]->GetName(), 
                *Waypoints[i]->GetActorLocation().ToString());
        }
    }
    UE_LOG(LogTemp, Warning, TEXT(""));

    // Branch 1: 3->4, 3->5, 4->6, 5->6
    if (Waypoints.Num() > 6)
    {
        Graph->AddEdge(Waypoints[2], Waypoints[3]); // 3->4
        Graph->AddEdge(Waypoints[2], Waypoints[4]); // 3->5
        Graph->AddEdge(Waypoints[3], Waypoints[5]); // 4->6
        Graph->AddEdge(Waypoints[4], Waypoints[5]); // 5->6
        UE_LOG(LogTemp, Warning, TEXT("Branch 1 Connections:"));
        UE_LOG(LogTemp, Warning, TEXT("  3->4: %s -> %s"), *Waypoints[2]->GetName(), *Waypoints[3]->GetName());
        UE_LOG(LogTemp, Warning, TEXT("  3->5: %s -> %s"), *Waypoints[2]->GetName(), *Waypoints[4]->GetName());
        UE_LOG(LogTemp, Warning, TEXT("  4->6: %s -> %s"), *Waypoints[3]->GetName(), *Waypoints[5]->GetName());
        UE_LOG(LogTemp, Warning, TEXT("  5->6: %s -> %s"), *Waypoints[4]->GetName(), *Waypoints[5]->GetName());
    }

    // Branch 2: 8->9, 8->10, 9->11, 10->11
    if (Waypoints.Num() > 10)
    {
        Graph->AddEdge(Waypoints[7], Waypoints[8]);  // 8->9
        Graph->AddEdge(Waypoints[7], Waypoints[9]);  // 8->10
        Graph->AddEdge(Waypoints[8], Waypoints[10]); // 9->11
        Graph->AddEdge(Waypoints[9], Waypoints[10]); // 10->11
        UE_LOG(LogTemp, Warning, TEXT("Branch 2 Connections:"));
        UE_LOG(LogTemp, Warning, TEXT("  8->9:  %s -> %s"), *Waypoints[7]->GetName(), *Waypoints[8]->GetName());
        UE_LOG(LogTemp, Warning, TEXT("  8->10: %s -> %s"), *Waypoints[7]->GetName(), *Waypoints[9]->GetName());
        UE_LOG(LogTemp, Warning, TEXT("  9->11: %s -> %s"), *Waypoints[8]->GetName(), *Waypoints[10]->GetName());
        UE_LOG(LogTemp, Warning, TEXT("  10->11: %s -> %s"), *Waypoints[9]->GetName(), *Waypoints[10]->GetName());
    }

    // Loop: 11->0
    if (Waypoints.Num() > 11)
    {
        Graph->AddEdge(Waypoints[10], Waypoints[0]); // 11->0
        UE_LOG(LogTemp, Warning, TEXT("Loop Connection:"));
        UE_LOG(LogTemp, Warning, TEXT("  11->0: %s -> %s (back to start)"), *Waypoints[10]->GetName(), *Waypoints[0]->GetName());
    }

    // Main loop: 0->1, 1->2, 5->6, 6->7
    if (Waypoints.Num() > 7)
    {
        Graph->AddEdge(Waypoints[0], Waypoints[1]); // 0->1
        Graph->AddEdge(Waypoints[1], Waypoints[2]); // 1->2
        Graph->AddEdge(Waypoints[5], Waypoints[6]); // 5->6
        Graph->AddEdge(Waypoints[6], Waypoints[7]); // 6->7
        UE_LOG(LogTemp, Warning, TEXT("Main Path Connections:"));
        UE_LOG(LogTemp, Warning, TEXT("  0->1: %s -> %s (Start)"), *Waypoints[0]->GetName(), *Waypoints[1]->GetName());
        UE_LOG(LogTemp, Warning, TEXT("  1->2: %s -> %s"), *Waypoints[1]->GetName(), *Waypoints[2]->GetName());
        UE_LOG(LogTemp, Warning, TEXT("  5->6: %s -> %s"), *Waypoints[5]->GetName(), *Waypoints[6]->GetName());
        UE_LOG(LogTemp, Warning, TEXT("  6->7: %s -> %s"), *Waypoints[6]->GetName(), *Waypoints[7]->GetName());
    }

    UE_LOG(LogTemp, Warning, TEXT("=== End of Waypoint Order ==="));
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