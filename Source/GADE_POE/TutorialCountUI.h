// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TutorialCountUI.generated.h"

/**
 * 
 */
UCLASS()
class GADE_POE_API UTutorialCountUI : public UUserWidget
{
	GENERATED_BODY()
	

public:
	// Initializes the tutorial UI when the game starts
	virtual void NativeConstruct() override;

	// Function bound to the "Continue" button
	UFUNCTION(BlueprintCallable)
	void OnContinueButtonPressed();



private:

	UPROPERTY(meta = (BindWidget))
	class UButton* ContinueButton; // The button to close the tutorial UI
};
