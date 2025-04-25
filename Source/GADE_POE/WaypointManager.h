#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CustomLinkedList.h"
#include "WaypointManager.generated.h"

UCLASS()
class GADE_POE_API AWaypointManager : public AActor
{
    GENERATED_BODY()

public:
    AWaypointManager();

    UPROPERTY(EditAnywhere, Category = "Waypoints")
    TArray<AActor*> Waypoints;

    UFUNCTION(BlueprintCallable, Category = "Waypoints")
    AActor* GetWaypoint(int32 Index);

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY()
    UCustomLinkedList* WaypointList;
};