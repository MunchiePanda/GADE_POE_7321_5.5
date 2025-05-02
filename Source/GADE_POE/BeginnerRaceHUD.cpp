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
}

void UBeginnerRaceHUD::UpdateLapCounter()
{
    if (PlayerHamster && GameState && LapCounter)
    {
        LapCounter->SetText(FText::FromString(FString::Printf(TEXT("Lap %d/%d"), PlayerHamster->CurrentLap, GameState->TotalLaps)));
    }
}
void UBeginnerRaceHUD::UpdatePositionDisplay()
{
    if (PlayerHamster && GameState && PositionDisplay)
    {
        TArray<FRacerLeaderboardEntry> Leaderboard = GameState->GetLeaderboard();
        int32 PlayerPosition = 0;
        for (const FRacerLeaderboardEntry& Entry : Leaderboard)
        {
            if (Entry.Racer == PlayerHamster)
            {
                PlayerPosition = Entry.Placement;
                break;
            }
        }
        PositionDisplay->SetText(FText::FromString(FString::Printf(TEXT("Position: %d"), PlayerPosition)));
    }
}