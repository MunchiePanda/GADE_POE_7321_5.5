// AdvancedRaceManager.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Graph.h"
#include "Waypoint.h"
#include "BiginnerRaceGameState.h"
#include "AdvancedRaceManager.generated.h"

class ABeginnerRaceGameState;
class AWaypoint;
class AGraph;
class AActor;

UCLASS()
class GADE_POE_API AAdvancedRaceManager : public AActor
{
    GENERATED_BODY()

public:
    AAdvancedRaceManager();

    UFUNCTION(BlueprintCallable)
    void InitializeTrack(AActor* RaceTrackActor, AGraph* InGraph);

    UFUNCTION(BlueprintCallable)
    void CollectWaypoints();

    UFUNCTION(BlueprintCallable)
    void PopulateGraph();

    UFUNCTION(BlueprintCallable, Category = "Waypoints")
    AWaypoint* GetWaypoint(int32 Index);

    UFUNCTION(BlueprintCallable, Category = "Waypoints")
    AGraph* GetGraph() const { return Graph; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
    TSubclassOf<class AWaypoint> WaypointClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Waypoint")
    TArray<AWaypoint*> Waypoints;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    AGraph* Graph;

    UPROPERTY()
    AActor* RaceTrack;

    UPROPERTY()
    ABeginnerRaceGameState* GameState;
};