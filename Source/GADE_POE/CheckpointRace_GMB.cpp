#include "CheckpointRace_GMB.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "CheckpointManager.h"
#include "RaceEndWidget.h"

ACheckpointRace_GMB::ACheckpointRace_GMB()
{
   
}

void ACheckpointRace_GMB::BeginPlay()
{
    Super::BeginPlay();

	// Show the tutorial UI
	ShowTutorial();

    // Spawn the race HUD
    if (RaceHUDWidgetClass)
    {
        RaceHUDWidget = CreateWidget<UUserWidget>(GetWorld(), RaceHUDWidgetClass);
        if (RaceHUDWidget)
        {
            RaceHUDWidget->AddToViewport();
        }
    }

    // Ensure input is enabled after restart
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC)
    {
        EnableInput(PC);
    }

	// Set the race state to running
    CheckpointManager = Cast<ACheckpointManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ACheckpointManager::StaticClass()));

    if (!CheckpointManager)
    {
        UE_LOG(LogTemp, Error, TEXT("CheckpointManager not found!"));
    }

}

void ACheckpointRace_GMB::CheckRaceStatus()
{
    if (CheckpointManager && CheckpointManager->GetRemainingCheckpoint() == 0 && CheckpointManager->GetCurrentLap() >= CheckpointManager->GetTotalLaps())
    {
        UE_LOG(LogTemp, Warning, TEXT("Race Finished!"));

        // Remove the RaceHUD
        if (RaceHUDWidget)
        {
            RaceHUDWidget->RemoveFromParent();
            RaceHUDWidget = nullptr;
        }

        // Pause the game
        UGameplayStatics::SetGamePaused(GetWorld(), true);

        // Show End Race UI
        if (RaceEndWidgetClass)
        {
            RaceEndWidget = CreateWidget<URaceEndWidget>(GetWorld(), RaceEndWidgetClass);
            if (RaceEndWidget)
            {
                RaceEndWidget->AddToViewport();
            }
        }
    }
}

void ACheckpointRace_GMB::ShowTutorial()
{
	// Spawn the tutorial UI
	if (TutorialWidgetClass)
	{
		UUserWidget* TutorialWidget = CreateWidget<UUserWidget>(GetWorld(), TutorialWidgetClass);
		if (TutorialWidget)
		{
			TutorialWidget->AddToViewport();
		}
	}
}

