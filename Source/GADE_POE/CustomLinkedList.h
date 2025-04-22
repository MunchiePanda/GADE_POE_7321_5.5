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
    void Add(AActor* Data);

    UFUNCTION(BlueprintCallable, Category = "LinkedList")
    AActor* GetAt(int32 Index) const;

    UFUNCTION(BlueprintCallable, Category = "LinkedList")
    int32 GetCount() const;

    UFUNCTION(BlueprintCallable, Category = "LinkedList")
    void Clear();
};