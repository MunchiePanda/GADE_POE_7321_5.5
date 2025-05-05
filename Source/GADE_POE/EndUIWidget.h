#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "BiginnerRaceGameState.h"
#include "RaceGameInstance.h"
#include "EndUIWidget.generated.h"

UCLASS()
class GADE_POE_API UEndUIWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

protected:
    UPROPERTY(meta = (BindWidget))
    UTextBlock* PositionText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* LeaderboardText;

    UPROPERTY(meta = (BindWidget))
    UButton* RestartButton;

    UPROPERTY(meta = (BindWidget))
    UButton* MainMenuButton;

    UPROPERTY(meta = (BindWidget))
    UButton* QuitButton;

    UFUNCTION()
    void OnRestartClicked();

    UFUNCTION()
	void OnMainMenuClicked(); // Function to go back to the main menu

    UFUNCTION()
	void OnQuitClicked(); // Function to quit the game

	UFUNCTION(BlueprintCallable, Category = "UI") // Function to update the UI with the player's position and leaderboard
    void UpdateUI();

private:
    UPROPERTY()
	ABeginnerRaceGameState* GameState; // The game state instance

    UPROPERTY()
	URaceGameInstance* GameInstance; // The game instance
};