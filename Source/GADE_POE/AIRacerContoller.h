#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "WaypointManager.h"
#include "CustomLinkedList.h"
#include "AIRacerContoller.generated.h"

UCLASS()
class GADE_POE_API AAIRacerContoller : public AAIController
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnWaypointReached(AActor* ReachedWaypoint);

protected:
    AActor* CurrentWaypoint;

    UPROPERTY()
    AWaypointManager* WaypointManager;

    UPROPERTY()
    UCustomLinkedList* LinkedList;

    FTimerHandle InitialMoveTimerHandle;

    void MoveToCurrentWaypoint();
    void DelayedMoveToCurrentWaypoint();
};