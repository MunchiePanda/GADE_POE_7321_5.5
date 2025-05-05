#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "BiginnerRaceGameState.h"
#include "RaceGameInstance.h"
#include "PauseMenuWidget.generated.h"

UCLASS()
class GADE_POE_API UPauseMenuWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

protected:
    UPROPERTY(meta = (BindWidget))
    UTextBlock* LeaderboardText;

    UPROPERTY(meta = (BindWidget))
    UButton* ResumeButton;

    UPROPERTY(meta = (BindWidget))
    UButton* RestartButton;

    UPROPERTY(meta = (BindWidget))
    UButton* MainMenuButton;

    UPROPERTY(meta = (BindWidget))
    UButton* QuitButton;

    UFUNCTION()
    void OnResumeClicked();

    UFUNCTION()
    void OnRestartClicked();

    UFUNCTION()
    void OnMainMenuClicked();

    UFUNCTION()
    void OnQuitClicked();

    UFUNCTION(BlueprintCallable, Category = "UI")
    void UpdateLeaderboard();

private:
    UPROPERTY()
    ABeginnerRaceGameState* GameState;

    UPROPERTY()
    URaceGameInstance* GameInstance;
};