// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CheckpointActor.generated.h"

UCLASS()
class GADE_POE_API ACheckpointActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACheckpointActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** Called when player overlaps with checkpoint */
	UFUNCTION()
	void OnPlayerEnterCheckpoint(AActor* OverlappedActor, AActor* OtherActor); // Function to call when player enters checkpoint

	void SetCheckpointState(bool bIsNextCheckpoint,bool BIsPassed);


	/** Indicator (e.g., floating ring or glow effect) */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* IndicatorMesh;

	UPROPERTY(EditAnywhere, Category = "Checkpoint Indicator")
	UMaterialInstance* YellowMaterial;

	UPROPERTY(EditAnywhere, Category = "Checkpoint Indicator")
	UMaterialInstance* GreenMaterial;

};
