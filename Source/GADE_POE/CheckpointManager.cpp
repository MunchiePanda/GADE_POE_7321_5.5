// Fill out your copyright notice in the Description page of Project Settings.


#include "CheckpointManager.h"
#include "CheckpointActor.h"

// Sets default values
ACheckpointManager::ACheckpointManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACheckpointManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACheckpointManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACheckpointManager::AddCheckpoint(ACheckpointActor* Checkpoint)
{
	if (Checkpoint)
	{
		CheckpointStack.Push(Checkpoint);
	}
}

void ACheckpointManager::PlayerReachedCheckpoint()
{
	ACheckpointActor* ReachedCheckpoint;
	if (CheckpointStack.Pop(ReachedCheckpoint))
	{
		UE_LOG(LogTemp, Warning, TEXT("Checkpoint Reached: %s"), *ReachedCheckpoint->GetName());
		ReachedCheckpoint->Destroy(); // Remove checkpoint from the level
	}
}

ACheckpointActor* ACheckpointManager::GetNextCheckpoint()
{
	ACheckpointActor* NextCheckpoint;
	if (CheckpointStack.Peek(NextCheckpoint))
	{
		return NextCheckpoint;
	}
	return nullptr;
}

