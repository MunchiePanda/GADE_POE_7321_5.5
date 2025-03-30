// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RaceEndWidget.generated.h"

/**
 * 
 */
UCLASS()
class GADE_POE_API URaceEndWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	virtual void NativeConstruct() override;
	UFUNCTION(BlueprintCallable)
	void OnRestartClicked();
	UFUNCTION(BlueprintCallable)
	void OnMainMenuClicked();

private:
	UPROPERTY()
	class UButton* RestartButton;
	UPROPERTY()
	class UButton* MainMenuButton;

	UPROPERTY()
	class UTextBlock* RaceTimeText;
};
