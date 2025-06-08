// PauseMenu_WB.h
// This widget class handles the pause menu functionality in the game,
// including options to restart, return to main menu, or quit the game.

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
    // Called after the widget is constructed
    virtual void NativeConstruct() override;

private:
    // Restarts the current level
    UFUNCTION()
	void RestartGame(); // Restarts the game

    // Returns to the main menu level
    UFUNCTION()
	void GoToMainMenu(); // Goes to the main menu

    // Exits the game application
    UFUNCTION()
	void QuitGame(); // Quits the game

    // Called when a button is hovered over
    UFUNCTION()
	void OnButtonHovered();

public:
    // Button to restart the current level
    UPROPERTY(meta = (BindWidget))
	class UButton* RestartButton; // The button to restart the game

    // Button to return to main menu
    UPROPERTY(meta = (BindWidget))
	class UButton* MainMenuButton; // The button to go to the main menu

    // Button to quit the game
    UPROPERTY(meta = (BindWidget))
	class UButton* QuitButton; // The button to quit the game

	UFUNCTION(BlueprintCallable)
	void TogglePauseMenu(); // Toggles the pause menu
};
