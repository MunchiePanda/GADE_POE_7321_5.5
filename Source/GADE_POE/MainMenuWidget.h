#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Slider.h"
#include "MainMenuWidget.generated.h"

UCLASS()
class GADE_POE_API UMainMenuWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    void OnMapOneClicked();

    UFUNCTION(BlueprintCallable)
    void OnMapTwoClicked();

    UFUNCTION(BlueprintCallable)
    void OnMapThreeClicked();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UUserWidget> LoadingScreenWidgetClass;

private:
    UPROPERTY()
    UUserWidget* LoadingScreenWidget;

    UPROPERTY()
    USlider* LoadingProgressSlider;

    void ShowLoadingScreen();
    void HideLoadingScreen();
    void LoadLevelAsync(const FName& LevelName);
    void UpdateLoadingProgress(float Progress);
};
