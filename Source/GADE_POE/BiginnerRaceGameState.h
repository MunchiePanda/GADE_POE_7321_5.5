// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "AIRacer.h"
#include "BiginnerRaceGameState.generated.h"

USTRUCT(BlueprintType)
struct FRacerLeaderboardEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    AActor* Racer; // Reference to the racer

    UPROPERTY(BlueprintReadOnly)
    FString RacerName; // Name of the racer

    UPROPERTY(BlueprintReadOnly)
    int32 Lap; // Current lap

    UPROPERTY(BlueprintReadOnly)
	int32 WaypointIndex; // Current waypoint index

    UPROPERTY(BlueprintReadOnly)
    int32 Placement; // Current placement
};

UCLASS()
class GADE_POE_API ABeginnerRaceGameState : public AGameStateBase
{
    GENERATED_BODY()
public:
    ABeginnerRaceGameState();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Leaderboard")
    void RegisterRacer(AActor* Racer); // Register the racer with the game state

    UFUNCTION(BlueprintCallable, Category = "Leaderboard")
    void UpdateRacerProgress(AActor* Racer, int32 Lap, int32 WaypointIndex); // Update the racer's progress

    UFUNCTION(BlueprintCallable, Category = "Leaderboard")
    TArray<FRacerLeaderboardEntry> GetLeaderboard() const; // Get the leaderboard

    UPROPERTY(BlueprintReadOnly, Category = "Leaderboard")
    TArray<FRacerLeaderboardEntry> Leaderboard; // Leaderboard

    UPROPERTY(BlueprintReadOnly, Category = "Race")
    int32 TotalLaps; // Total number of laps

    UPROPERTY(BlueprintReadOnly, Category = "Race")
    int32 TotalWaypoints; // Total number of waypoints

    UPROPERTY(BlueprintReadOnly, Category = "Race")
    bool bRaceFinished; // Whether the race has finished
private:
    void UpdateLeaderboard(); // Update the leaderboard
};
