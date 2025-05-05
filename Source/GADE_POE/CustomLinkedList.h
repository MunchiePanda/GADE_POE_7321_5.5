#pragma once

#include "CoreMinimal.h"
#include "TempLinkedList.h"
#include "CustomLinkedList.generated.h"

UCLASS()
class GADE_POE_API UCustomLinkedList : public UObject
{
    GENERATED_BODY()

private:
    TempLinkedList<AActor*> WaypointList;

public:
    UCustomLinkedList();

    UFUNCTION(BlueprintCallable, Category = "LinkedList")
    void Add(AActor* Data); // Add an element to the end of the list

    UFUNCTION(BlueprintCallable, Category = "LinkedList")
    AActor* GetAt(int32 Index) const; // Get an element at a specific index

    UFUNCTION(BlueprintCallable, Category = "LinkedList")
    int32 GetCount() const; // Get the number of elements

    UFUNCTION(BlueprintCallable, Category = "LinkedList")
    void Clear(); // Clear the list

    // NEW FUNCTIONS
    UFUNCTION(BlueprintCallable, Category = "LinkedList")
    AActor* GetFirst() const; // Get the first element of the list

    UFUNCTION(BlueprintCallable, Category = "LinkedList")
    AActor* GetNext(AActor* Current) const; // Get the next element of the list
};
