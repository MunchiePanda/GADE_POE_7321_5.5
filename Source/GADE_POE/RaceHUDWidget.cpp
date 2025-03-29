#include "RaceHUDWidget.h"
#include "Components/TextBlock.h"
#include "PlayerHamster.h"
#include "CheckpointManager.h"
#include "EngineUtils.h"
void URaceHUDWidget::SetRaceStats(float Speed, float TimeElapsed, int32 CurrentLap, int32 TotalLaps)
{
    CurrentSpeed = Speed;
    ElapsedTime = TimeElapsed;
    LapNumber = CurrentLap;
    TotalLapCount = TotalLaps;
}

void URaceHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

	ElapsedTime += InDeltaTime;

}

void URaceHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

    for(TActorIterator<ACheckpointManager> It(GetWorld()); It; ++It) // Find CheckpointManager in the level
	{
		CheckpointManagerClass = *It;
        break;
	}

	for (TActorIterator<APlayerHamster> It(GetWorld()); It; ++It) // Find PlayerHamster in the level
	{
		PlayerHamsterClass = *It;
		break;
	}
}

FText URaceHUDWidget::GetSpeedText() const
{
    if (PlayerHamsterClass)
    {
        return FText::FromString(FString::Printf(TEXT("Speed: %.1f km/h"), PlayerHamsterClass->GetSpeed()));
        
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
    if (CheckpointManagerClass)
    {
        int32 RemainingCheckpoints = CheckpointManagerClass->GetRemainingCheckpoint();
        int32 CurrentLap = CheckpointManagerClass->GetCurrentLap();
        int32 TotalLaps = CheckpointManagerClass->GetTotalLaps();

        return FText::FromString(FString::Printf(TEXT("Lap %d/%d \n Checkpoints Left: %d"), CurrentLap, TotalLaps, RemainingCheckpoints));
    }
    return FText::FromString(TEXT("Lap 0/0 | Checkpoints Left: 0"));
}


