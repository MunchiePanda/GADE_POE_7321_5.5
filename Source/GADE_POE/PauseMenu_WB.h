// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenu_WB.generated.h"

/**
 * 
 */
UCLASS()
class GADE_POE_API UPauseMenu_WB : public UUserWidget
{
	GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;

private:
    UFUNCTION()
	void RestartGame(); // Restarts the game

    UFUNCTION()
	void GoToMainMenu(); // Goes to the main menu

    UFUNCTION()
	void QuitGame(); // Quits the game
public:
    UPROPERTY(meta = (BindWidget))
	class UButton* RestartButton; // The button to restart the game

    UPROPERTY(meta = (BindWidget))
	class UButton* MainMenuButton; // The button to go to the main menu

    UPROPERTY(meta = (BindWidget))
	class UButton* QuitButton; // The button to quit the game
};
