#pragma once

#include "CoreMinimal.h"

/**
 * A custom Queue implementation using TLinkedList (FIFO structure)
 */
template <typename T>
class DialogueQueueTemp
{
public:
    DialogueQueueTemp() : Head(nullptr), Tail(nullptr) {}
    ~DialogueQueueTemp()
    {
        Clear();
    }

    /** Adds an item to the queue */
    void Enqueue(const T& Item)
    {
        TNode* NewNode = new TNode(Item);
        if (!Tail) // If queue is empty, set both head and tail
        {
            Head = Tail = NewNode;
        }
        else
        {
            Tail->Next = NewNode;
            Tail = NewNode;
        }

        UE_LOG(LogTemp, Warning, TEXT("Enqueued item: %s"), *Item.Name);
    }


    /** Removes and returns the front item of the queue */
    bool Dequeue(T& OutItem)
    {
        if (!Head) return false; // Queue is empty

        OutItem = Head->Data;
        TNode* OldHead = Head;
        Head = Head->Next;

        if (!Head) // If queue is now empty, reset tail
        {
            Tail = nullptr;
        }

        delete OldHead;
        return true;
    }

    /** Returns the front item without removing it */
    bool Peek(T& OutItem) const
    {
        if (!Head) return false;
        OutItem = Head->Data;
        return true;
    }

    /** Checks if the queue is empty */
    bool IsEmpty() const
    {
        return Head == nullptr;
    }

    /** Clears the queue */
    void Clear()
    {
        while (Head)
        {
            TNode* Temp = Head;
            Head = Head->Next;
            delete Temp;
        }
        Tail = nullptr;
    }

    int32 GetSize() const
    {
        int32 Count = 0;
        TNode* Temp = Head;
        while (Temp)
        {
            Count++;
            Temp = Temp->Next;
        }
        return Count;
    }


private:
    /** Internal node structure */
    struct TNode
    {
        T Data;
        TNode* Next;
        TNode(const T& InData) : Data(InData), Next(nullptr) {}
    };

    TNode* Head; // Front of the queue
    TNode* Tail; // Back of the queue
};
