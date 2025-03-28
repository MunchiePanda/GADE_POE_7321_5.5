// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Blueprint/UserWidget.h"
#include "MainMenu_GMB.generated.h"

/**
 * 
 */
UCLASS()
class GADE_POE_API AMainMenu_GMB : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> MainMenuClass;

private:
	UUserWidget* CurrentWidget;
	
};
