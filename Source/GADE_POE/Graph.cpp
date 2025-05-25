#include "Graph.h"

AGraph::AGraph()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AGraph::AddNode(AActor* Waypoint)
{
    if (Waypoint && !FindNode(Waypoint))
    {
        Nodes.Add(FGraphNode(Waypoint));
    }
}

void AGraph::AddEdge(AActor* From, AActor* To)
{
    TNode<FGraphNode>* FromNode = FindNode(From);
    TNode<FGraphNode>* ToNode = FindNode(To);
    if (FromNode && ToNode)
    {
        FromNode->Data.Neighbors.Add(To);
    }
}

TArray<AActor*> AGraph::GetNeighbors(AActor* Waypoint)
{
    TArray<AActor*> Result; // Temporary TArray for Blueprint output
    TNode<FGraphNode>* Node = FindNode(Waypoint);
    if (Node)
    {
        TNode<AActor*>* Current = Node->Data.Neighbors.GetHead();
        while (Current)
        {
            Result.Add(Current->Data);
            Current = Current->Next;
        }
    }
    return Result;
}

TNode<FGraphNode>* AGraph::FindNode(AActor* Waypoint)
{
    TNode<FGraphNode>* Current = Nodes.GetHead();
    while (Current)
    {
        if (Current->Data.Waypoint == Waypoint)
            return Current;
        Current = Current->Next;
    }
    return nullptr;
}