/**
 AIRacerController Class
 *
 This class handles the AI logic and navigation for racing characters.
 It manages waypoint following, path finding, and racing behavior decisions.
 The controller supports both simple waypoint following and advanced graph-based navigation.
 */

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

/** Debug visualization console variable for AI racer paths */
static TAutoConsoleVariable<int32> CVarShowDebugPath(
    TEXT("AIRacer.ShowDebugPath"),
    0,
    TEXT("Show debug visualization of AI racer paths"),
    ECVF_Default);

// Forward declarations
class AWaypointManager;
class UCustomLinkedList;
class AActor;
class ABeginnerRaceGameState;
class AAdvancedRaceManager;
class AGraph;
class AWaypoint;

UCLASS()
class GADE_POE_API AAIRacerContoller : public AAIController
{
    GENERATED_BODY()

public:
    /** Constructor - Initializes controller components and settings */
    AAIRacerContoller();

    /** Called when the game starts */
    virtual void BeginPlay() override;
    
    /** Called every frame to update AI behavior */
    virtual void Tick(float DeltaTime) override;
    
    /** Handles logic when a waypoint is reached */
    void OnWaypointReached(AActor* ReachedWaypoint);

    /** Initializes the navigation graph for advanced pathfinding */
    UFUNCTION(BlueprintCallable)
    void InitializeGraph(AGraph* InGraph);

    /** Returns the current waypoint target */
    UFUNCTION(BlueprintCallable, Category = "Navigation")
    AWaypoint* GetCurrentWaypoint() const { return CurrentWaypoint; }

protected:
    /** Manager for linear waypoint navigation */
    UPROPERTY()
    AWaypointManager* WaypointManager;

    /** Linked list of waypoints for simple navigation */
    UPROPERTY()
    UCustomLinkedList* LinkedList;

    /** Graph for advanced navigation pathfinding */
    UPROPERTY()
    AGraph* Graph;

    /** Manager for advanced race features and waypoints */
    UPROPERTY()
    AAdvancedRaceManager* AdvancedRaceManager;

    /** Current waypoint target */
    UPROPERTY()
    AWaypoint* CurrentWaypoint;

    /** Reference to game state for race management */
    UPROPERTY()
    ABeginnerRaceGameState* GameState;

    /** Timer for delayed initial movement */
    FTimerHandle InitialMoveTimerHandle;
    
    /** Timer for initialization checks */
    FTimerHandle InitTimerHandle;

    /** Flag indicating if controller is fully initialized */
    bool bInitialized;

    /** Toggle between simple waypoint and graph-based navigation */
    UPROPERTY(EditAnywhere, Category = "Navigation")
    bool bUseGraphNavigation;

    /** Initiates movement to waypoint after delay */
    void DelayedMoveToCurrentWaypoint();
    
    /** Handles actual movement logic to current waypoint */
    void MoveToCurrentWaypoint();

    /** Determines which navigation system to use */
    void DetermineNavigationType();
    
    /** Sets up graph-based navigation */
    void InitializeGraphNavigation();
    
    /** Sets up simple waypoint navigation */
    void InitializeWaypointNavigation();
    
    /** Initializes the racer's starting position */
    void InitializeRacerPosition();
};