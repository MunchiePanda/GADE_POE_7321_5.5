#pragma once
#include "CoreMinimal.h"
#include "AIController.h"
#include "AIRacer.h"
#include "Kismet/GameplayStatics.h"
#include "WaypointManager.h"
#include "CustomLinkedList.h"
#include "Graph.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "BiginnerRaceGameState.h"
#include "AdvancedRaceManager.h"
#include "AIRacerContoller.generated.h"


class AWaypointManager;
class UCustomLinkedList;
class AGraph;
class AActor;
class ABeginnerRaceGameState;

UCLASS()
class GADE_POE_API AAIRacerContoller : public AAIController
{
    GENERATED_BODY()

public:
    AAIRacerContoller();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    void OnWaypointReached(AActor* ReachedWaypoint);

    UFUNCTION(BlueprintCallable)
    void InitializeGraph(AGraph* InGraph);

protected:
    UPROPERTY()
    AWaypointManager* WaypointManager;

    UPROPERTY()
    UCustomLinkedList* LinkedList;

    UPROPERTY()
    AGraph* Graph;

    UPROPERTY()
    AAdvancedRaceManager* AdvancedRaceManager;

    UPROPERTY()
    AActor* CurrentWaypoint;

    UPROPERTY()
    ABeginnerRaceGameState* GameState;

    FTimerHandle InitialMoveTimerHandle;

    bool bInitialized;

    UPROPERTY(EditAnywhere, Category = "Navigation")
    bool bUseGraphNavigation;

    void DelayedMoveToCurrentWaypoint();
    void MoveToCurrentWaypoint();
};