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