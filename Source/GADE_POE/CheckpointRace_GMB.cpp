#include "CheckpointRace_GMB.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

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

