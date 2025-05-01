#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AIRacerContoller.generated.h"

class AWaypointManager;
class UCustomLinkedList;
class AActor;

UCLASS()
class GADE_POE_API AAIRacerContoller : public AAIController
{
    GENERATED_BODY()

public:
    AAIRacerContoller();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    void OnWaypointReached(AActor* ReachedWaypoint);
protected:
    UPROPERTY()
    AWaypointManager* WaypointManager;

    UPROPERTY()
    UCustomLinkedList* LinkedList;

    UPROPERTY()
    AActor* CurrentWaypoint;

    FTimerHandle InitialMoveTimerHandle;

    bool bInitialized;

    void DelayedMoveToCurrentWaypoint();
    
    void MoveToCurrentWaypoint();
};