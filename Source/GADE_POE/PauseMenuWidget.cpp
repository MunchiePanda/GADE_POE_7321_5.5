#include "PauseMenuWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "BiginnerRaceGameState.h"
#include "RaceGameInstance.h"



void UPauseMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    GameState = Cast<ABeginnerRaceGameState>(GetWorld()->GetGameState());
    GameInstance = Cast<URaceGameInstance>(GetGameInstance());

	// Check if GameState and GameInstance are valid
    if (GameState)
    {
        UpdateLeaderboard();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PauseMenuWidget: GameState not found"));
    }

	//bind all buttons
    if (ResumeButton)
    {
        ResumeButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnResumeClicked);
    }
    if (RestartButton)
    {
        RestartButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnRestartClicked);
    }
    if (MainMenuButton)
    {
        MainMenuButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnMainMenuClicked);
    }
    if (QuitButton)
    {
        QuitButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnQuitClicked);
    }

    // Pause game and show cursor
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PlayerController)
    {
        PlayerController->SetPause(true);
        PlayerController->bShowMouseCursor = true;
        PlayerController->SetInputMode(FInputModeUIOnly());
    }

}

void UPauseMenuWidget::NativeDestruct()
{
    if (ResumeButton)
    {
        ResumeButton->OnClicked.RemoveDynamic(this, &UPauseMenuWidget::OnResumeClicked);
    }
    if (RestartButton)
    {
        RestartButton->OnClicked.RemoveDynamic(this, &UPauseMenuWidget::OnRestartClicked);
    }
    if (MainMenuButton)
    {
        MainMenuButton->OnClicked.RemoveDynamic(this, &UPauseMenuWidget::OnMainMenuClicked);
    }
    if (QuitButton)
    {
        QuitButton->OnClicked.RemoveDynamic(this, &UPauseMenuWidget::OnQuitClicked);
    }

    Super::NativeDestruct();
}

void UPauseMenuWidget::OnResumeClicked()
{
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PlayerController)
    {
        PlayerController->SetPause(false);
        PlayerController->bShowMouseCursor = false;
        PlayerController->SetInputMode(FInputModeGameOnly());
    }
    RemoveFromParent();
}

void UPauseMenuWidget::OnRestartClicked() //Restart the game 
{
    UGameplayStatics::OpenLevel(GetWorld(), FName("BeginnerMap"));
    
}

void UPauseMenuWidget::OnMainMenuClicked()
{
	UGameplayStatics::OpenLevel(GetWorld(), FName("StartingMenu")); // Load the main menu level
}

void UPauseMenuWidget::OnQuitClicked() //Quit the game
{ 
    UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, false);
}

void UPauseMenuWidget::UpdateLeaderboard()
{
	// Update the leaderboard text 
    if (GameState && LeaderboardText)
    {
        TArray<FRacerLeaderboardEntry> Rankings = GameState->GetLeaderboard();
        FString LeaderboardString = TEXT("Leaderboard:\n");
        for (const FRacerLeaderboardEntry& Entry : Rankings)
        {
            LeaderboardString += FString::Printf(TEXT("%d. %s (Lap %d, Waypoint %d)\n"),
                Entry.Placement, *Entry.RacerName, Entry.Lap, Entry.WaypointIndex);
        }
        LeaderboardText->SetText(FText::FromString(LeaderboardString));
    }
}