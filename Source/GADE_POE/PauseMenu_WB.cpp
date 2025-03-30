// Fill out your copyright notice in the Description page of Project Settings.


#include "PauseMenu_WB.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

void UPauseMenu_WB::NativeConstruct()
{
	Super::NativeConstruct();

	//bind buttons to on-click events
    if (RestartButton) RestartButton->OnClicked.AddDynamic(this, &UPauseMenu_WB::RestartGame);
    if (MainMenuButton) MainMenuButton->OnClicked.AddDynamic(this, &UPauseMenu_WB::GoToMainMenu);
    if (QuitButton) QuitButton->OnClicked.AddDynamic(this, &UPauseMenu_WB::QuitGame);

    // Pause the game when the menu is opened
    //UGameplayStatics::SetGamePaused(GetWorld(), true);
}

void UPauseMenu_WB::RestartGame()
{
	FName CurrentLevel = *GetWorld()->GetName(); // Get the current level name
	UGameplayStatics::OpenLevel(GetWorld(), CurrentLevel); // Restart the current level
}

void UPauseMenu_WB::GoToMainMenu()
{
	UGameplayStatics::OpenLevel(GetWorld(), "StartingMenu"); // Go to the main menu
}

void UPauseMenu_WB::QuitGame()
{
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController(); 
	// Get the player controller
    if (PlayerController) 
    {
		UKismetSystemLibrary::QuitGame(GetWorld(), PlayerController, EQuitPreference::Quit, false); // Quit the game
    }
}

void UPauseMenu_WB::TogglePauseMenu()
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (!PlayerController) return;

    bool bIsPaused = UGameplayStatics::IsGamePaused(World);

    if (bIsPaused)
    {
        // Unpause the game and remove menu
        RemoveFromParent();
        UGameplayStatics::SetGamePaused(World, false);
        PlayerController->SetInputMode(FInputModeGameOnly()); // Restore game input
        PlayerController->bShowMouseCursor = false;
    }
	if (!bIsPaused)
    {
        // Pause the game and show menu
        AddToViewport();
        UGameplayStatics::SetGamePaused(World, true);
        PlayerController->SetInputMode(FInputModeUIOnly()); // UI-only input
        PlayerController->bShowMouseCursor = true;
    }
}
