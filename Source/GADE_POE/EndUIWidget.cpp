#include "EndUIWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PlayerHamster.h"

void UEndUIWidget::NativeConstruct()
{
    Super::NativeConstruct();

	GameState = Cast<ABeginnerRaceGameState>(GetWorld()->GetGameState()); // Get the game state instance
	GameInstance = Cast<URaceGameInstance>(GetGameInstance()); // Get the game instance

	// Check if GameState and GameInstance are valid
    if (GameState)
    {
        UpdateUI();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("EndUIWidget: GameState not found"));
    } 

	// Bind button click events 
    if (RestartButton)
    {
        RestartButton->OnClicked.AddDynamic(this, &UEndUIWidget::OnRestartClicked);
    }
    if (MainMenuButton)
    {
        MainMenuButton->OnClicked.AddDynamic(this, &UEndUIWidget::OnMainMenuClicked);
    }
    if (QuitButton)
    {
        QuitButton->OnClicked.AddDynamic(this, &UEndUIWidget::OnQuitClicked);
    }

	// Set the mouse cursor to be visible and set input mode to UI only
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PlayerController)
    {
        PlayerController->bShowMouseCursor = true;
        PlayerController->SetInputMode(FInputModeUIOnly());
    }
}

void UEndUIWidget::NativeDestruct()
{
	// Unbind button click events 
    if (RestartButton)
    {
        RestartButton->OnClicked.RemoveDynamic(this, &UEndUIWidget::OnRestartClicked);
    }
    if (MainMenuButton)
    {
        MainMenuButton->OnClicked.RemoveDynamic(this, &UEndUIWidget::OnMainMenuClicked);
    }
    if (QuitButton)
    {
        QuitButton->OnClicked.RemoveDynamic(this, &UEndUIWidget::OnQuitClicked);
    }

    Super::NativeDestruct();
}

void UEndUIWidget::OnRestartClicked() 
{
        UE_LOG(LogTemp, Warning, TEXT("EndUIWidget: CurrentLevelName not set, defaulting to BeginnerMap"));
		UGameplayStatics::OpenLevel(GetWorld(), FName("BeginnerMap")); // Restart the game by opening the level
}

// Function to go back to the main menu
void UEndUIWidget::OnMainMenuClicked()
{
    UGameplayStatics::OpenLevel(GetWorld(), FName("StartingMenu")); 
}

// Function to quit the game
void UEndUIWidget::OnQuitClicked()
{
    UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, false);
}

// Function to update the UI with the player's position and leaderboard 
void UEndUIWidget::UpdateUI()
{
    if (GameState && LeaderboardText)
    {
		// Get the leaderboard from the game state and format it as a string
        TArray<FRacerLeaderboardEntry> Rankings = GameState->GetLeaderboard();
        FString LeaderboardString = TEXT("Leaderboard:\n");
		// Sort the leaderboard by placement
        for (int32 i = 0; i < Rankings.Num(); ++i) 
        {
            const FRacerLeaderboardEntry& Entry = Rankings[i];
            FString EntryString = FString::Printf(TEXT("%d. %s (Lap %d, Waypoint %d)\n"),
                Entry.Placement, *Entry.RacerName, Entry.Lap, Entry.WaypointIndex);

            // Highlight top 3
            if (Entry.Placement == 1)
            {
                EntryString = FString::Printf(TEXT("<Gold>%s</>"), *EntryString);
            }
            else if (Entry.Placement == 2)
            {
                EntryString = FString::Printf(TEXT("<Silver>%s</>"), *EntryString);
            }
            else if (Entry.Placement == 3)
            {
                EntryString = FString::Printf(TEXT("<Bronze>%s</>"), *EntryString);
            }

            LeaderboardString += EntryString;
        }
		LeaderboardText->SetText(FText::FromString(LeaderboardString)); // Set the leaderboard text
    }

	// Update the player's position in the leaderboard
    if (GameState && PositionText)
    {
        TArray<FRacerLeaderboardEntry> Rankings = GameState->GetLeaderboard();
        int32 PlayerRank = -1;
		// Find the player's rank in the leaderboard
        for (const FRacerLeaderboardEntry& Entry : Rankings)
        {
            APlayerHamster* Racer = Cast<APlayerHamster>(Entry.Racer);
            if (Racer)
            {
                PlayerRank = Entry.Placement;
                break;
            }
        }

        FString PositionString;
		// Format the player's position string based on their rank
        switch (PlayerRank)
        {
        case 1: PositionString = TEXT("1st Place"); break;
        case 2: PositionString = TEXT("2nd Place"); break;
        case 3: PositionString = TEXT("3rd Place"); break;
        default: PositionString = PlayerRank > 0 ? FString::Printf(TEXT("%dth Place"), PlayerRank) : TEXT("Unknown Place"); break;
        }
		PositionText->SetText(FText::FromString(PositionString)); // Set the player's position text
    }
}