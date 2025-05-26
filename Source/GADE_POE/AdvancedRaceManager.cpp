// Fill out your copyright notice in the Description page of Project Settings.

#include "AdvancedRaceManager.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AAdvancedRaceManager::AAdvancedRaceManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void AAdvancedRaceManager::InitializeTrack(AActor* RaceTrackActor, AGraph* InGraph)
{
	RaceTrack = RaceTrackActor;
	Graph = InGraph;
}

void AAdvancedRaceManager::CollectWaypoints()
{
    if (!Graph)
        return;

    Waypoints.Empty();
    // Option 1: Collect manually placed waypoints in the level
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWaypoint::StaticClass(), FoundActors);
    for (AActor* Actor : FoundActors)
    {
        if (AWaypoint* Waypoint = Cast<AWaypoint>(Actor))
        {
            Waypoints.Add(Waypoint);
            Graph->AddNode(Waypoint);
        }
    }

    // Option 2: Programmatic spawning (uncomment if needed)
    /*
    if (WaypointClass && RaceTrack)
    {
        // Example: Spawn waypoints at predefined locations
        TArray<FVector> WaypointLocations = {
            FVector(0, 0, 100),   // A
            FVector(1000, 0, 100), // B
            FVector(2000, 0, 100), // C
            FVector(1000, 500, 100), // D (branch 1)
            // Add more for branch 2 and main loop
        };
        for (const FVector& Location : WaypointLocations)
        {
            AWaypoint* Waypoint = GetWorld()->SpawnActor<AWaypoint>(WaypointClass, Location, FRotator::ZeroRotator);
            if (Waypoint)
            {
                Waypoints.Add(Waypoint);
                Graph->AddNode(Waypoint);
            }
        }
    }
    */
}

void AAdvancedRaceManager::PopulateGraph()
{
    if (!Graph || Waypoints.Num() < 4)
        return;

    // Example edge setup (adjust for your track)
    // Main loop: A->B->C->J
    // Branch 1: B->D->C
    // Branch 2: F->H->I
    Graph->AddEdge(Waypoints[0], Waypoints[1]); // A->B
    Graph->AddEdge(Waypoints[1], Waypoints[2]); // B->C
    Graph->AddEdge(Waypoints[1], Waypoints[3]); // B->D (branch 1)
    Graph->AddEdge(Waypoints[3], Waypoints[2]); // D->C
    // Add edges for branch 2 and main loop
}

// Called when the game starts or when spawned
void AAdvancedRaceManager::BeginPlay()
{
	Super::BeginPlay();
	
}

