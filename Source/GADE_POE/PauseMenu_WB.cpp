// Fill out your copyright notice in the Description page of Project Settings.


#include "PauseMenu_WB.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "SFXManager.h"

void UPauseMenu_WB::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind button click events to their respective functions
	if (RestartButton) RestartButton->OnClicked.AddDynamic(this, &UPauseMenu_WB::RestartGame);
	if (MainMenuButton) MainMenuButton->OnClicked.AddDynamic(this, &UPauseMenu_WB::GoToMainMenu);
	if (QuitButton) QuitButton->OnClicked.AddDynamic(this, &UPauseMenu_WB::QuitGame);

	// Bind button hover events
	if (RestartButton) RestartButton->OnHovered.AddDynamic(this, &UPauseMenu_WB::OnButtonHovered);
	if (MainMenuButton) MainMenuButton->OnHovered.AddDynamic(this, &UPauseMenu_WB::OnButtonHovered);
	if (QuitButton) QuitButton->OnHovered.AddDynamic(this, &UPauseMenu_WB::OnButtonHovered);

	// Pause the game when the menu is opened
	//UGameplayStatics::SetGamePaused(GetWorld(), true);
}

void UPauseMenu_WB::OnButtonHovered()
{
	// Play hover sound
	if (ASFXManager* SFXManager = ASFXManager::GetInstance(GetWorld()))
	{
		SFXManager->PlayButtonHoverSound();
	}
}

void UPauseMenu_WB::RestartGame()
{
	// Play click sound
	if (ASFXManager* SFXManager = ASFXManager::GetInstance(GetWorld()))
	{
		SFXManager->PlayButtonClickSound();
	}

	// Get current level name and restart it
	FName CurrentLevel = *GetWorld()->GetName();
	UGameplayStatics::OpenLevel(GetWorld(), CurrentLevel);
}

void UPauseMenu_WB::GoToMainMenu()
{
	// Play click sound
	if (ASFXManager* SFXManager = ASFXManager::GetInstance(GetWorld()))
	{
		SFXManager->PlayButtonClickSound();
	}

	// Load the starting menu level
	UGameplayStatics::OpenLevel(GetWorld(), "StartingMenu");
}

void UPauseMenu_WB::QuitGame()
{
	// Play click sound
	if (ASFXManager* SFXManager = ASFXManager::GetInstance(GetWorld()))
	{
		SFXManager->PlayButtonClickSound();
	}

	// Get player controller and quit the game
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (PlayerController)
	{
		UKismetSystemLibrary::QuitGame(GetWorld(), PlayerController, EQuitPreference::Quit, false);
	}
}

void UPauseMenu_WB::TogglePauseMenu()
{
	if (IsVisible())
	{
		// Play menu close sound
		if (ASFXManager* SFXManager = ASFXManager::GetInstance(GetWorld()))
		{
			SFXManager->PlayMenuCloseSound();
		}

		RemoveFromParent();
		UGameplayStatics::SetGamePaused(GetWorld(), false);
	}
	else
	{
		// Play menu open sound
		if (ASFXManager* SFXManager = ASFXManager::GetInstance(GetWorld()))
		{
			SFXManager->PlayMenuOpenSound();
		}

		AddToViewport();
		UGameplayStatics::SetGamePaused(GetWorld(), true);
	}
}
