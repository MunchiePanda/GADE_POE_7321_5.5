#include "StartingMenuWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/KismetSystemLibrary.h"
void UStartingMenuWidget::OnStartClicked()
{
    SwitchToMainMenu();
}

void UStartingMenuWidget::OnExitClicked()
{

    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    UKismetSystemLibrary::QuitGame(GetWorld(), PlayerController, EQuitPreference::Quit, false);
}

void UStartingMenuWidget::SwitchToMainMenu()
{
    if (MainMenuWidgetClass)
    {
        UUserWidget* MainMenuWidget = CreateWidget<UUserWidget>(GetWorld(), MainMenuWidgetClass);
        if (MainMenuWidget)
        {
            MainMenuWidget->AddToViewport();
            RemoveFromParent();
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("MainMenuWidgetClass is not set."));
    }
}

void UStartingMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Play the fade-in animation
    if (FadeInAnimation)
    {
        PlayAnimation(FadeInAnimation);
    }
}
