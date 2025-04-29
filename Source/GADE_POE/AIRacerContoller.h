// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "WaypointManager.h"
#include "CustomLinkedList.h"
#include "AIRacerContoller.generated.h"

/**
 * 
 */
UCLASS()
class GADE_POE_API AAIRacerContoller : public AAIController
{
	GENERATED_BODY()
public:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnWaypointReached(AActor* ReachedWaypoint); // Called when a waypoint is reached

protected:
	AActor* CurrentWaypoint; // Current waypoint the racer is moving towards

    UPROPERTY()
	AWaypointManager* WaypointManager; // Reference to the WaypointManager

    UPROPERTY()
    UCustomLinkedList* LinkedList; // Reference to the linked list

    void MoveToCurrentWaypoint(); // Move to the current waypoint
};
