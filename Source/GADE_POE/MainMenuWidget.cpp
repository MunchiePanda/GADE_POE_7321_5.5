#include "MainMenuWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "Components/Slider.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "TimerManager.h"

void UMainMenuWidget::OnMapOneClicked()
{
    LoadLevelAsync(FName("CheckpointMap"));
}

void UMainMenuWidget::OnMapTwoClicked()
{
    LoadLevelAsync(FName("BeginnerMap"));
}

void UMainMenuWidget::OnMapThreeClicked()
{
    LoadLevelAsync(FName("AdvancedMap"));
}

void UMainMenuWidget::ShowLoadingScreen()
{
    if (LoadingScreenWidgetClass)
    {
        LoadingScreenWidget = CreateWidget<UUserWidget>(GetWorld(), LoadingScreenWidgetClass);
        if (LoadingScreenWidget)
        {
            LoadingScreenWidget->AddToViewport();
            LoadingProgressSlider = Cast<USlider>(LoadingScreenWidget->GetWidgetFromName(TEXT("LoadingProgressSlider")));
        }
    }
}

void UMainMenuWidget::HideLoadingScreen()
{
    if (LoadingScreenWidget)
    {
        LoadingScreenWidget->RemoveFromParent();
        LoadingScreenWidget = nullptr;
        LoadingProgressSlider = nullptr;
    }
}

void UMainMenuWidget::LoadLevelAsync(const FName& LevelName)
{
    ShowLoadingScreen();

    FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
    Streamable.RequestAsyncLoad(LevelName.ToString(), FStreamableDelegate::CreateLambda([this, LevelName]()
        {
            HideLoadingScreen();
            UGameplayStatics::OpenLevel(this, LevelName);
        }));
}

void UMainMenuWidget::UpdateLoadingProgress(float Progress)
{
    if (LoadingProgressSlider)
    {
        LoadingProgressSlider->SetValue(Progress);
    }
}
