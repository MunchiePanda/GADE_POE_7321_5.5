#include "RaceHUDWidget.h"
#include "Components/TextBlock.h"

void URaceHUDWidget::UpdateSpeed(float Speed)
{
    if (SpeedText)
    {
        SpeedText->SetText(FText::FromString(FString::Printf(TEXT("Speed: %.1f km/h"), Speed)));
    }
}

void URaceHUDWidget::UpdateTime(float TimeElapsed)
{
    if (TimeText)
    {
        int32 Minutes = FMath::FloorToInt(TimeElapsed / 60);
        int32 Seconds = FMath::FloorToInt(FMath::Fmod(TimeElapsed, 60));

        TimeText->SetText(FText::FromString(FString::Printf(TEXT("Time: %02d:%02d"), Minutes, Seconds)));
    }
}

void URaceHUDWidget::UpdateLaps(int32 CurrentLap, int32 TotalLaps)
{
    if (LapText)
    {
        LapText->SetText(FText::FromString(FString::Printf(TEXT("Lap: %d / %d"), CurrentLap, TotalLaps)));
    }
}
