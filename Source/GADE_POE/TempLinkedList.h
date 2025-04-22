#pragma once

#include "CoreMinimal.h"

// Debug macro to verify inclusion
#define TEMPLINKEDLIST_DEBUG 1

template<typename T>
struct TNode
{
    T Data;
    TNode* Next;

    TNode() : Data(), Next(nullptr) {}
    TNode(const T& InData) : Data(InData), Next(nullptr) {}
};

template<typename T>
class TempLinkedList
{
private:
    TNode<T>* Head;
    int32 Count;

public:
    TempLinkedList() : Head(nullptr), Count(0) {}

    ~TempLinkedList()
    {
        Clear();
    }

    void Add(const T& Data)
    {
        TNode<T>* NewNode = new TNode<T>(Data);
        if (!Head)
        {
            Head = NewNode;
        }
        else
        {
            TNode<T>* Current = Head;
            while (Current->Next)
            {
                Current = Current->Next;
            }
            Current->Next = NewNode;
        }
        Count++;
    }

    T GetAt(int32 Index) const
    {
        if (Index < 0 || Index >= Count)
        {
            UE_LOG(LogTemp, Warning, TEXT("TLinkedList: Index out of range: %d"), Index);
            return T();
        }

        TNode<T>* Current = Head;
        for (int32 i = 0; i < Index; i++)
        {
            Current = Current->Next;
        }
        return Current->Data;
    }

    int32 GetCount() const
    {
        return Count;
    }

    void Clear()
    {
        TNode<T>* Current = Head;
        while (Current)
        {
            TNode<T>* Next = Current->Next;
            delete Current;
            Current = Next;
        }
        Head = nullptr;
        Count = 0;
    }
};