#include "CustomLinkedList.h"
#include "TempLinkedList.h"

UCustomLinkedList::UCustomLinkedList()
{
}

void UCustomLinkedList::Add(AActor* Data)
{
    WaypointList.Add(Data);
}

AActor* UCustomLinkedList::GetAt(int32 Index) const
{
    return WaypointList.GetAt(Index);
}

int32 UCustomLinkedList::GetCount() const
{
    return WaypointList.GetCount();
}

void UCustomLinkedList::Clear()
{
    WaypointList.Clear();
}

AActor* UCustomLinkedList::GetFirst() const
{
    return WaypointList.GetAt(0);
}

AActor* UCustomLinkedList::GetNext(AActor* Current) const
{
    int32 Count = WaypointList.GetCount();
    for (int32 i = 0; i < Count; ++i)
    {
        if (WaypointList.GetAt(i) == Current)
        {
            int32 NextIndex = (i + 1) % Count; // Loop back to start
            return WaypointList.GetAt(NextIndex);
        }
    }

    return nullptr;
}
