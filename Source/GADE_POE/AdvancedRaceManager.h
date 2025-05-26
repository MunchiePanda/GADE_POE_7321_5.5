// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Graph.h"
#include "Waypoint.h"
#include "AdvancedRaceManager.generated.h"

UCLASS()
class GADE_POE_API AAdvancedRaceManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAdvancedRaceManager();
    UFUNCTION(BlueprintCallable)
    void InitializeTrack(AActor* RaceTrackActor, AGraph* InGraph);

    UFUNCTION(BlueprintCallable)
    void CollectWaypoints();

    UFUNCTION(BlueprintCallable)
    void PopulateGraph();

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    AGraph* Graph;

    UPROPERTY()
    AActor* RaceTrack; // Static mesh actor

    UPROPERTY(EditAnywhere, Category = "Waypoint")
    TSubclassOf<AWaypoint> WaypointClass;

    UPROPERTY(EditAnywhere, Category = "Waypoint")
    TArray<AWaypoint*> Waypoints; // Manual or programmatic waypoints

};
