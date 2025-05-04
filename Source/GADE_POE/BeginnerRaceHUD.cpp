// Fill out your copyright notice in the Description page of Project Settings.


#include "BeginnerRaceHUD.h"
#include "Components/TextBlock.h"
#include "PlayerHamster.h"
#include "Kismet/GameplayStatics.h"

void UBeginnerRaceHUD::NativeConstruct()
{
    Super::NativeConstruct();

    PlayerHamster = Cast<APlayerHamster>(GetOwningPlayerPawn());
    GameState = Cast<ABeginnerRaceGameState>(GetWorld()->GetGameState());

    if (!LapCounter)
    {
        UE_LOG(LogTemp, Error, TEXT("LapCounter not found in WBP_HUD!"));
    }
    if (!PositionDisplay)
    {
        UE_LOG(LogTemp, Error, TEXT("PositionDisplay not found in WBP_HUD!"));
    }
}

void UBeginnerRaceHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    UpdateLapCounter();
	UpdatePositionDisplay();
}

void UBeginnerRaceHUD::UpdateLapCounter()
{
    if (PlayerHamster && GameState && LapCounter)
    {
        LapCounter->SetText(FText::FromString(FString::Printf(TEXT("Lap %d/%d"), PlayerHamster->CurrentLap, GameState->TotalLaps)));
        UE_LOG(LogTemp, Warning, TEXT("Lap %d/%d"), PlayerHamster->CurrentLap, GameState->TotalLaps);
    }
}

// Function to update the position display 
void UBeginnerRaceHUD::UpdatePositionDisplay() // Update the position display
{
    if (PlayerHamster && GameState && PositionDisplay)
    {
		// Get the leaderboard from the game state and find the player's position 
        TArray<FRacerLeaderboardEntry> Leaderboard = GameState->GetLeaderboard();
        int32 PlayerPosition = 0;

        for (const FRacerLeaderboardEntry& Entry : Leaderboard) // Loop through the leaderboard
        {
            if (Entry.Racer == PlayerHamster)
            {
                PlayerPosition = Entry.Placement;
                break;
            }
        }
        PositionDisplay->SetText(FText::FromString(FString::Printf(TEXT("Position: %d"), PlayerPosition)));
		UE_LOG(LogTemp, Warning, TEXT("Position: %d"), PlayerPosition);
    }
}

FText UBeginnerRaceHUD::GetSpeedText() const // Function to get the speed text
{
    if (PlayerHamster)
    {
        return FText::FromString(FString::Printf(TEXT("Speed: %.1f km/h"), PlayerHamster->GetSpeed()));

    }
    return FText::FromString(TEXT("Speed: 0 km/h"));
}