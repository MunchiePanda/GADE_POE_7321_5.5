#include "StartingMenuWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "SFXManager.h"

void UStartingMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Play menu open sound when constructed
    if (ASFXManager* SFXManager = ASFXManager::GetInstance(GetWorld()))
    {
        SFXManager->PlayMenuOpenSound();
    }

    // Play the fade-in animation
    if (FadeInAnimation)
    {
        PlayAnimation(FadeInAnimation);
    }
}

void UStartingMenuWidget::OnStartClicked()
{
    // Play click sound
    if (ASFXManager* SFXManager = ASFXManager::GetInstance(GetWorld()))
    {
        SFXManager->PlayButtonClickSound();
    }

    SwitchToMainMenu();
}

void UStartingMenuWidget::OnExitClicked()
{
    // Play click sound
    if (ASFXManager* SFXManager = ASFXManager::GetInstance(GetWorld()))
    {
        SFXManager->PlayButtonClickSound();
    }

    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    UKismetSystemLibrary::QuitGame(GetWorld(), PlayerController, EQuitPreference::Quit, false);
}

void UStartingMenuWidget::OnButtonHovered()
{
    // Play hover sound
    if (ASFXManager* SFXManager = ASFXManager::GetInstance(GetWorld()))
    {
        SFXManager->PlayButtonHoverSound();
    }
}

void UStartingMenuWidget::SwitchToMainMenu()
{
    if (MainMenuWidgetClass)
    {
        // Play menu close sound for current menu
        if (ASFXManager* SFXManager = ASFXManager::GetInstance(GetWorld()))
        {
            SFXManager->PlayMenuCloseSound();
        }

        UUserWidget* MainMenuWidget = CreateWidget<UUserWidget>(GetWorld(), MainMenuWidgetClass);
        if (MainMenuWidget)
        {
            // Play menu open sound for new menu
            if (ASFXManager* SFXManager = ASFXManager::GetInstance(GetWorld()))
            {
                SFXManager->PlayMenuOpenSound();
            }

            MainMenuWidget->AddToViewport();
            RemoveFromParent();
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("MainMenuWidgetClass is not set."));
    }
}
