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
    return FText::FromString(FString::Printf(TEXT("Lap: %d / %d"), LapNumber, TotalLapCount));
}

FText URaceHUDWidget::GetCheckpointText() const
{
    if (CheckpointManagerClass)
    {
        int32 RemainingCheckpoints = CheckpointManagerClass->GetRemainingCheckpoint();
        UE_LOG(LogTemp, Warning, TEXT("Remaining Checkpoints: %d"), RemainingCheckpoints);
        return FText::FromString(FString::Printf(TEXT("Checkpoints Left: %d"), RemainingCheckpoints));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Checkpoint Manager Not Found"));
        return FText::FromString(TEXT("Checkpoint Manager Not Found"));
    }

    //return FText::FromString(TEXT("Checkpoints Left: 0"));
}
