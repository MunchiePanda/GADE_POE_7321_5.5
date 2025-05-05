#include "MainMenuWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "Components/Slider.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "TimerManager.h"

void UMainMenuWidget::OnMapOneClicked()
{
    LoadLevelAsync(FName("CheckpointDialogueLVL")); // Open the dialogue level and then load the race level
}

void UMainMenuWidget::OnMapTwoClicked()
{
	LoadLevelAsync(FName("BeginnerDialogueLVL")); // Open the dialogue level for the beginner race and then load the race level
}

void UMainMenuWidget::OnMapThreeClicked()
{
	LoadLevelAsync(FName("AdvancedDialogueLVL")); // Load the advanced map
}

void UMainMenuWidget::ShowLoadingScreen()
{
    if (LoadingScreenWidgetClass) // Check if the class is set in the editor
    {
        LoadingScreenWidget = CreateWidget<UUserWidget>(GetWorld(), LoadingScreenWidgetClass); // Create the widget
        if (LoadingScreenWidget) // Check if the widget was created successfully
        {
            LoadingScreenWidget->AddToViewport();
            LoadingProgressSlider = Cast<USlider>(LoadingScreenWidget->GetWidgetFromName(TEXT("LoadingProgressSlider")));
        }
    }
}

void UMainMenuWidget::HideLoadingScreen() // Hide the loading screen
{
    if (LoadingScreenWidget) // Check if the widget exists
    {
        LoadingScreenWidget->RemoveFromParent();
        LoadingScreenWidget = nullptr;
        LoadingProgressSlider = nullptr;
    }
}

void UMainMenuWidget::LoadLevelAsync(const FName& LevelName) // Load the level
{
	if (LevelName.IsNone()) // Check if the level name is empty
    {
        UE_LOG(LogTemp, Error, TEXT("LoadLevelAsync called with an EMPTY level name!"));
        return; // Exit if the level name is empty
    }

    ShowLoadingScreen(); // Show the loading screen

    // Delay opening the level by 2 seconds (simulating async load)
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, LevelName]() // Create a lambda function
        {
            HideLoadingScreen();
            UGameplayStatics::OpenLevel(this, LevelName);
        }, 2.0f, false);  // Adjust delay time if needed

    UE_LOG(LogTemp, Warning, TEXT("Starting async load for level: %s"), *LevelName.ToString());
}

void UMainMenuWidget::UpdateLoadingProgress(float Progress) // Update the loading progress
{
    if (LoadingProgressSlider) // Check if the slider exists
    {
        LoadingProgressSlider->SetValue(Progress);
    }
}
