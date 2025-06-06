// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CheckStackTemp.h"
#include "CheckpointActor.h"
#include "CheckpointManager.generated.h"

UCLASS()
class GADE_POE_API ACheckpointManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACheckpointManager();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Stack storing all checkpoints */
	CheckStackTemp <ACheckpointActor*> CheckpointStack; // Stack of checkpoints

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override; 

	/** Adds a checkpoint to the stack */
	UFUNCTION(BlueprintCallable, Category = "Checkpoints")
	void AddCheckpoint(ACheckpointActor* Checkpoint);

	/** Called when the player reaches a checkpoint */
	UFUNCTION(BlueprintCallable, Category = "Checkpoints")
	void PlayerReachedCheckpoint(); // Player reached a checkpoint

	/** Gets the next checkpoint */
	UFUNCTION(BlueprintCallable, Category = "Checkpoints")
	ACheckpointActor* GetNextCheckpoint(); // Get the next checkpoint

	/** Resets the checkpoints */
	UFUNCTION(BlueprintCallable, Category = "Checkpoints")
	void ResetCheckpoints();

	UFUNCTION(BlueprintCallable, Category = "Checkpoints")
	void DebugCheckpointStatus(); // Debug checkpoints
	 
	UFUNCTION(BlueprintCallable, Category = "Checkpoints") 
	int32 GetRemainingCheckpoint() const; // Get the number of checkpoints reached

	UFUNCTION(BlueprintCallable, Category = "Checkpoints")
	int32 GetCurrentLap() const { return CurrentLap; } // Get the current lap

	float GetRemainingTime() const {return RemainingTime;} // Get the remaining time

	UFUNCTION(BlueprintCallable, Category = "Checkpoints")
	int32 GetTotalLaps() const { return TotalLaps; } // Get the total laps

	UPROPERTY(EditAnywhere, Category = "Timer")
	float InitialTime = 20.0f; // Initial time in seconds

	UPROPERTY(EditAnywhere, Category = "Timer")
	float TimePerCheckpoint = 20.0f; // Time added

	float RemainingTime;

	
	void HandleTimerExpiry();


	UPROPERTY(EditAnywhere, Category = "Sound")
	USoundBase* CheckpointReachedSound; // Sound effect for che

private:
	bool bCheckpointCleared = false; // Flag to prevent multiple calls

	int32 TotalLaps = 2; // Set total laps
	int32 CurrentLap = 1; // Start at lap 1
	TArray<ACheckpointActor*> AllCheckpoints; // Store checkpoints for resetting
};
