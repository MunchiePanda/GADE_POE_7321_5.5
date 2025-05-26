#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TempHashMap.h"
#include "Graph.generated.h"

USTRUCT()
struct FGraphNode
{
    GENERATED_BODY()
    UPROPERTY()
    AActor* Waypoint;
    TempLinkedList<AActor*> Neighbors;

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
    TArray<AActor*> GetNeighbors(AActor* Waypoint);

private:
    TempHashMap<AActor*, FGraphNode> Nodes;
};