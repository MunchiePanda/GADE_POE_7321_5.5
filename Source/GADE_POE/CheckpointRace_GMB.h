// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Dialogue_Data.h"
#include "RaceHUDWidget.h"
#include "CheckpointRace_GMB.generated.h"

/**
 * 
 */
class URaceHUDWidget;
UCLASS()
class GADE_POE_API ACheckpointRace_GMB : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACheckpointRace_GMB();

	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD")
	TSubclassOf<UUserWidget> RaceHUDWidgetClass;

private:
	UPROPERTY()
	UDialogue_Data* DialogueData;

	UPROPERTY()
	URaceHUDWidget* RaceHUDWidget;
	
};
