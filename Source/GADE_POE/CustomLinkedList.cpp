#include "CustomLinkedList.h"
#include "TempLinkedList.h"

UCustomLinkedList::UCustomLinkedList()
{
}

// Add an element to the end of the list
void UCustomLinkedList::Add(AActor* Data) 
{
    WaypointList.Add(Data);
}

// Add an element to the end of the list
AActor* UCustomLinkedList::GetAt(int32 Index) const
{
    return WaypointList.GetAt(Index);
}

// Get the number of elements in the list
int32 UCustomLinkedList::GetCount() const
{
    return WaypointList.GetCount(); // Return the number of elements
}

// Clear the list
void UCustomLinkedList::Clear()
{
    WaypointList.Clear();
}

// Get the first element
AActor* UCustomLinkedList::GetFirst() const
{
    return WaypointList.GetAt(0);
}

AActor* UCustomLinkedList::GetNext(AActor* Current) const
{
	// Check if the current element is valid
    int32 Count = WaypointList.GetCount();
    for (int32 i = 0; i < Count; ++i)
    {
        if (WaypointList.GetAt(i) == Current)
        {
            int32 NextIndex = (i + 1) % Count; // Loop back to start
            return WaypointList.GetAt(NextIndex);
        }
    }

    return nullptr; // Return nullptr if the current element is not found
}
