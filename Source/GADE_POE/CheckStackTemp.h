#pragma once

#include "CoreMinimal.h"

template <typename T>
class GADE_POE_API CheckStackTemp
{
private:
    struct Node
    {
        T Data;
        Node* Next;
        Node(const T& InData) : Data(InData), Next(nullptr) {}
    };

    Node* Top;
    int32 StackSize;

public:
    CheckStackTemp() : Top(nullptr), StackSize(0) {}
    ~CheckStackTemp() { Clear(); }

    /** Push an item onto the stack */
    void Push(const T& Item)
    {
        Node* NewNode = new Node(Item);
        NewNode->Next = Top;
        Top = NewNode;
        StackSize++;
    }

    /** Pops the top item from the stack */
    bool Pop(T& OutItem)
    {
        if (IsEmpty())
            return false;

        Node* TempNode = Top; // Store the top node
        OutItem = Top->Data;
        Top = Top->Next; // Move the top pointer
        delete TempNode;
        StackSize--;
        return true;
    }

    /** Peek at the top item without removing it */
    bool Peek(T& OutItem) const
    {
        if (IsEmpty())
            return false;

        OutItem = Top->Data;
        return true;
    }

    /** Check if the stack is empty */
    bool IsEmpty() const
    {
        return Top == nullptr;
    }

    /** Clear the stack */
    void Clear()
    {
        while (!IsEmpty())
        {
            Node* TempNode = Top;
            Top = Top->Next;
            delete TempNode;
        }
        StackSize = 0;
    }

    /** Get the number of elements in the stack */
    int32 Size() const
    {
        return StackSize;
    }
};
