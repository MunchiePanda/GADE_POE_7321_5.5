#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AIRacerContoller.generated.h"

class AWaypointManager;
class UCustomLinkedList;
class AActor;
class ABeginnerRaceGameState;

UCLASS()
class GADE_POE_API AAIRacerContoller : public AAIController // Inherit from AAIController (please ignore bad spelling I was tired)
{
    GENERATED_BODY()

public:
    AAIRacerContoller();

    virtual void BeginPlay() override; 
    virtual void Tick(float DeltaTime) override;
    void OnWaypointReached(AActor* ReachedWaypoint); // Triggered when a waypoint is reached

protected:
    UPROPERTY()
    AWaypointManager* WaypointManager; // Reference to the WaypointManager

    UPROPERTY()
    UCustomLinkedList* LinkedList; // Reference to the LinkedList

    UPROPERTY()
	AActor* CurrentWaypoint; // Current waypoint actor

    UPROPERTY()
	ABeginnerRaceGameState* GameState; // Reference to the game state

    FTimerHandle InitialMoveTimerHandle; // Timer handle

	bool bInitialized; // Flag to check if the AI has been initialized

    void DelayedMoveToCurrentWaypoint(); // Delayed movement to the current waypoint

	void MoveToCurrentWaypoint(); // Move to the current waypoint
};