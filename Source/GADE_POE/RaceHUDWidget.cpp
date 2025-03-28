#include "RaceHUDWidget.h"
#include "Components/TextBlock.h"
#include "PlayerHamster.h"

void URaceHUDWidget::SetRaceStats(float Speed, float TimeElapsed, int32 CurrentLap, int32 TotalLaps)
{
    CurrentSpeed = Speed;
    ElapsedTime = TimeElapsed;
    LapNumber = CurrentLap;
    TotalLapCount = TotalLaps;
}

FText URaceHUDWidget::GetSpeedText() const
{
    if (PlayerHamsterClass)
    {
        APlayerHamster* PlayerHamster = Cast<APlayerHamster>(PlayerHamsterClass->GetDefaultObject());
        if (PlayerHamster)
        {
            return FText::FromString(FString::Printf(TEXT("Speed: %.1f km/h"), PlayerHamster->GetSpeed()));
        }
    }
    return FText::FromString(TEXT("Speed: 0 km/h"));
}

FText URaceHUDWidget::GetTimeText() const
{
    int32 Minutes = FMath::FloorToInt(ElapsedTime / 60);
    int32 Seconds = FMath::FloorToInt(FMath::Fmod(ElapsedTime, 60));
    return FText::FromString(FString::Printf(TEXT("Time: %02d:%02d"), Minutes, Seconds));
}

FText URaceHUDWidget::GetLapText() const
{
    return FText::FromString(FString::Printf(TEXT("Lap: %d / %d"), LapNumber, TotalLapCount));
}
