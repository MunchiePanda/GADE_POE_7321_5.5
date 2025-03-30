// Fill out your copyright notice in the Description page of Project Settings.


#include "TutorialCountUI.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void UTutorialCountUI::NativeConstruct()
{
    Super::NativeConstruct();

    // Pause the game at the start
    UGameplayStatics::SetGamePaused(GetWorld(), true);

    // Bind Continue button click event
    if (ContinueButton)
    {
        ContinueButton->OnClicked.AddDynamic(this, &UTutorialCountUI::OnContinueButtonPressed);
    }
}


void UTutorialCountUI::OnContinueButtonPressed()
{
    // Unpause the game
    UGameplayStatics::SetGamePaused(GetWorld(), false);

    // Hide the tutorial UI
    RemoveFromParent();
}

