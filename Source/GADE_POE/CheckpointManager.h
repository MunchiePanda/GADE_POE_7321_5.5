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
	CheckStackTemp <ACheckpointActor*> CheckpointStack;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** Adds a checkpoint to the stack */
	UFUNCTION(BlueprintCallable, Category = "Checkpoints")
	void AddCheckpoint(ACheckpointActor* Checkpoint);

	/** Called when the player reaches a checkpoint */
	UFUNCTION(BlueprintCallable, Category = "Checkpoints")
	void PlayerReachedCheckpoint();

	/** Gets the next checkpoint */
	UFUNCTION(BlueprintCallable, Category = "Checkpoints")
	ACheckpointActor* GetNextCheckpoint();

	UFUNCTION(BlueprintCallable, Category = "Checkpoints")
	void DebugCheckpointStatus();

private:
	bool bCheckpointCleared = false;

};
