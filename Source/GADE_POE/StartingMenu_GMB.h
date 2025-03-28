// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Blueprint/UserWidget.h"
#include "StartingMenu_GMB.generated.h"

/**
 * 
 */
UCLASS()
class GADE_POE_API AStartingMenu_GMB : public AGameModeBase
{
	GENERATED_BODY()

public :
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> StartingMenuClass;

	virtual void BeginPlay() override;

private:
	UUserWidget* CurrentWidget;
	
};
