#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TempLinkedList.h"
#include "Graph.generated.h"

USTRUCT()
struct FGraphNode
{
    GENERATED_BODY()
    UPROPERTY()
    AActor* Waypoint; // Waypoint Actor (e.g., placed on spline)
    TempLinkedList<AActor*> Neighbors; // List of neighboring waypoints

    FGraphNode() : Waypoint(nullptr) {}
    FGraphNode(AActor* InWaypoint) : Waypoint(InWaypoint) {}
};

UCLASS()
class GADE_POE_API AGraph : public AActor
{
    GENERATED_BODY()
public:
    AGraph();

    UFUNCTION(BlueprintCallable)
    void AddNode(AActor* Waypoint);

    UFUNCTION(BlueprintCallable)
    void AddEdge(AActor* From, AActor* To);

    UFUNCTION(BlueprintCallable)
    TArray<AActor*> GetNeighbors(AActor* Waypoint); // TArray for Blueprint compatibility

private:
    TempLinkedList<FGraphNode> Nodes; // List of graph nodes

    TNode<FGraphNode>* FindNode(AActor* Waypoint); // Find node by waypoint
};