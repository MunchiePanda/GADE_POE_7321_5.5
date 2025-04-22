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

protected:
    UPROPERTY(EditAnywhere, Category = "Waypoints")
    TArray<AActor*> Waypoints;

    UPROPERTY(VisibleAnywhere, Category = "Waypoints")
    UCustomLinkedList* WaypointList;

    virtual void BeginPlay() override;

public:
    UFUNCTION(BlueprintCallable, Category = "Waypoints")
    AActor* GetWaypoint(int32 Index) const;

    UFUNCTION(BlueprintCallable, Category = "Waypoints")
    int32 GetWaypointCount() const;
};