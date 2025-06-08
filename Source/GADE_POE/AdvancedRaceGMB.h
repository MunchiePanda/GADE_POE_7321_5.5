// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AdvancedRaceManager.h"
#include "Graph.h"
#include "Blueprint/UserWidget.h"
#include "AdvancedRaceGMB.generated.h"

/**
 * 
 */
UCLASS()
class GADE_POE_API AAdvancedRaceGMB : public AGameModeBase
{
	GENERATED_BODY()

public:
	AAdvancedRaceGMB();

	virtual void BeginPlay() override;

	// Advanced race specific properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race Setup")
	TSubclassOf<AGraph> GraphClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race Setup")
	TSubclassOf<AAdvancedRaceManager> RaceManagerClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race Setup")
	AActor* RaceTrack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> RaceHUDClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> RaceEndWidgetClass;

protected:
	// Reference to the race manager
	UPROPERTY()
	AAdvancedRaceManager* RaceManager;

	// Reference to the graph
	UPROPERTY()
	AGraph* Graph;

	// Initialize race components
	void InitializeRaceComponents();

	// Setup race UI
	void SetupRaceUI();
};
