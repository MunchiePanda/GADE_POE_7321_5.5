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


    // Find all spectators in the level
    for (TActorIterator<ASpectator> It(GetWorld()); It; ++It)
    {
        Spectators.Add(*It);
    }

    // Test Disappointed state after 10 seconds (simulating a race event)
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
        {
            for (ASpectator* Spectator : Spectators)
            {
                if (Spectator)
                {
                    Spectator->BeDisappointed();
                }
            }
        }, 10.0f, false);
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