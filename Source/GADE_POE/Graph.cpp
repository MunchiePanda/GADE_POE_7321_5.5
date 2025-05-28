#include "Graph.h"

AGraph::AGraph()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AGraph::AddNode(AActor* Waypoint)
{
    if (Waypoint)
    {
        FGraphNode* ExistingNode = Nodes.Get(Waypoint);
        if (!ExistingNode)
        {
            Nodes.Add(Waypoint, FGraphNode(Waypoint));
        }
    }
}

void AGraph::AddEdge(AActor* From, AActor* To)
{
    FGraphNode* FromNode = Nodes.Get(From);
    FGraphNode* ToNode = Nodes.Get(To);
    if (FromNode && ToNode)
    {
        FromNode->Neighbors.Add(To);
    }
}

TArray<AActor*> AGraph::GetNeighbors(AActor* Waypoint)
{
    TArray<AActor*> Result;
    FGraphNode* Node = Nodes.Get(Waypoint);
    if (Node)
    {
        TNode<AActor*>* Current = Node->Neighbors.GetHead();
        while (Current)
        {
            Result.Add(Current->Data);
            Current = Current->Next;
        }
    }
    return Result;
}

TArray<AActor*> AGraph::GetAllKeys(TArray<AActor*>& OutWaypoints)
{
   TArray<AActor*> Result;
    OutWaypoints = Result;
    Nodes.GetAllKeys(Result);
    return Result;
}