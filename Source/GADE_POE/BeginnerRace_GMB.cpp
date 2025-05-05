#include "BeginnerRace_GMB.h"
#include "Spectator.h"
#include "TimerManager.h"
#include "Blueprint/UserWidget.h"
#include "EngineUtils.h" // For TActorIterator
ABeginnerRace_GMB::ABeginnerRace_GMB()
{
}

void ABeginnerRace_GMB::BeginPlay()
{
    Super::BeginPlay();

	// Show the tutorial UI
	ShowTutorial();
}

void ABeginnerRace_GMB::ShowTutorial() 
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