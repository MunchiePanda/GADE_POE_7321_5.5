#include "Graph.h"

AGraph::AGraph()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AGraph::AddNode(AActor* Waypoint)
{
    if (!Waypoint || !Waypoint->IsValidLowLevel())
    {
        UE_LOG(LogTemp, Error, TEXT("Graph::AddNode - NULL or invalid waypoint"));
        return;
    }
    if (PendingRemoval.Contains(Waypoint))
    {
        UE_LOG(LogTemp, Warning, TEXT("Graph::AddNode - Waypoint %s is pending removal"), *Waypoint->GetName());
        return;
    }
    if (!Nodes.Get(Waypoint))
    {
        Nodes.Add(Waypoint, FGraphNode(Waypoint));
        UE_LOG(LogTemp, Log, TEXT("Graph::AddNode - Added waypoint: %s"), *Waypoint->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Graph::AddNode - Waypoint already exists: %s"), *Waypoint->GetName());
    }
}

void AGraph::AddEdge(AActor* From, AActor* To)
{
    if (!From || !From->IsValidLowLevel() || !To || !To->IsValidLowLevel())
    {
        UE_LOG(LogTemp, Error, TEXT("Graph::AddEdge - Invalid actor pointers"));
        return;
    }
    if (PendingRemoval.Contains(From) || PendingRemoval.Contains(To))
    {
        UE_LOG(LogTemp, Warning, TEXT("Graph::AddEdge - Waypoint(s) pending removal: From=%s, To=%s"),
            *From->GetName(), *To->GetName());
        return;
    }
    FGraphNode* FromNode = Nodes.Get(From);
    FGraphNode* ToNode = Nodes.Get(To);
    if (FromNode && ToNode)
    {
        FromNode->Neighbors.Add(To);
        UE_LOG(LogTemp, Log, TEXT("Graph::AddEdge - Added edge from %s to %s"), *From->GetName(), *To->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Graph::AddEdge - Invalid nodes: From=%s, To=%s"),
            FromNode ? *From->GetName() : TEXT("null"), ToNode ? *To->GetName() : TEXT("null"));
    }
}

TArray<AActor*> AGraph::GetNeighbors(AActor* Waypoint)
{
    TArray<AActor*> Result;

    if (!Waypoint || !Waypoint->IsValidLowLevel())
    {
        UE_LOG(LogTemp, Error, TEXT("Graph::GetNeighbors - Waypoint is NULL or invalid!"));
        return Result;
    }
    if (PendingRemoval.Contains(Waypoint))
    {
        UE_LOG(LogTemp, Warning, TEXT("Graph::GetNeighbors - Waypoint %s is pending removal"), *Waypoint->GetName());
        return Result;
    }

    FGraphNode* Node = Nodes.Get(Waypoint);
    if (!Node)
    {
        UE_LOG(LogTemp, Error, TEXT("Graph::GetNeighbors - Waypoint %s not found in Nodes map!"), *Waypoint->GetName());
        return Result;
    }

    TNode<AActor*>* Current = Node->Neighbors.GetHead();
    while (Current)
    {
        if (Current->Data && Current->Data->IsValidLowLevel() && !PendingRemoval.Contains(Current->Data))
        {
            Result.Add(Current->Data);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Graph::GetNeighbors - Invalid or pending removal neighbor in %s"), *Waypoint->GetName());
        }
        Current = Current->Next;
    }

    return Result;
}

TArray<AActor*> AGraph::GetAllKeys(TArray<AActor*>& OutWaypoints)
{
    TArray<AActor*> Result;
    OutWaypoints.Empty();
    Nodes.GetAllKeys(Result);
    for (AActor* Waypoint : Result)
    {
        if (Waypoint && Waypoint->IsValidLowLevel() && !PendingRemoval.Contains(Waypoint))
        {
            OutWaypoints.Add(Waypoint);
        }
    }
    return OutWaypoints;
}

void AGraph::RemoveNode(AActor* Waypoint)
{
    if (!Waypoint || !Waypoint->IsValidLowLevel())
    {
        UE_LOG(LogTemp, Warning, TEXT("Graph::RemoveNode - Invalid or null waypoint"));
        return;
    }

    PendingRemoval.Add(Waypoint);
    Nodes.Remove(Waypoint);
    UE_LOG(LogTemp, Log, TEXT("Graph::RemoveNode - Removed waypoint: %s from Nodes map"), *Waypoint->GetName());

    TArray<AActor*> AllKeys;
    Nodes.GetAllKeys(AllKeys);
    for (AActor* Key : AllKeys)
    {
        if (Key && Key->IsValidLowLevel() && !PendingRemoval.Contains(Key))
        {
            if (FGraphNode* Node = Nodes.Get(Key))
            {
                int32 OldCount = Node->Neighbors.GetCount();
                Node->Neighbors.Remove([this, Waypoint](const AActor* N) {
                    return N && N->IsValidLowLevel() && N == Waypoint;
                    });
                if (Node->Neighbors.GetCount() < OldCount)
                {
                    UE_LOG(LogTemp, Log, TEXT("Graph::RemoveNode - Removed %s from %s's Neighbors list"),
                        *Waypoint->GetName(), *Key->GetName());
                }
            }
        }
    }

    PendingRemoval.Remove(Waypoint);
}