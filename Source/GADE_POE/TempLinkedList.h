#pragma once

#include "CoreMinimal.h"

// Debug macro to verify inclusion
#define TEMPLINKEDLIST_DEBUG 1

template<typename T> // Templated linked list
struct TNode
{
	T Data; // Data of type T
    TNode* Next; // Pointer to the next node

	TNode() : Data(), Next(nullptr) {} // Default constructor
    TNode(const T& InData) : Data(InData), Next(nullptr) {} // Constructor
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
		TNode<T>* NewNode = new TNode<T>(Data); // Create a new node with the given data
        if (!Head)
        {
			Head = NewNode; // If the list is empty, set the head to the new node
        }
        else
        {
			TNode<T>* Current = Head; // Start from the head
            while (Current->Next) // Find the last node
            { 
                Current = Current->Next; 
            }
            Current->Next = NewNode;
        }
		Count++; // Increment the count
    }

	T GetAt(int32 Index) const // Get the data at a specific index
    {
        if (Index < 0 || Index >= Count) 
        {
            UE_LOG(LogTemp, Warning, TEXT("TLinkedList: Index out of range: %d"), Index);
			return T(); // Return default value if index is out of range
        }

		// Traverse to the specified index
        TNode<T>* Current = Head;
        for (int32 i = 0; i < Index; i++)
        {
            Current = Current->Next;
        }
        return Current->Data;
    }

	int32 GetCount() const // Get the number of elements in the list
    {
        return Count;
    }

    void Clear() // Clear the list
    {
        // Traverse the list and delete each node 
        TNode<T>* Current = Head;
		// Delete each node in the list
        while (Current)
        {
            TNode<T>* Next = Current->Next;
            delete Current;
            Current = Next;
        }
        Head = nullptr; // Set the head to nullptr
        Count = 0; // Set the count to 0
    }
};