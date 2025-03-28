// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogueWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"

void UDialogueWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind button click event
    if (NextButton)
    {
        NextButton->OnClicked.AddDynamic(this, &UDialogueWidget::OnNextButtonClicked);
    }

    // Load dialogue data
    DialogueData = NewObject<UDialogue_Data>();
    if (DialogueData->LoadDialogue("GADE_TEST.json"))
    {
        OnNextButtonClicked();  // Show first dialogue automatically
    }
}

void UDialogueWidget::DisplayDialogue(FDialogue_Item DialogueItem)
{
    if (SpeakerNameText)
    {
        SpeakerNameText->SetText(FText::FromString(DialogueItem.SpeakerName));
    }

    if (DialogueTextBlock)
    {
        DialogueTextBlock->SetText(FText::FromString(DialogueItem.DialogueText));
    }

    if (SpeakerPortraitImage)
    {
        // Load portrait (you'll need a texture asset path in Blueprint)
    }
}


void UDialogueWidget::OnNextButtonClicked()
{
    if (DialogueData && !DialogueData->IsQueueEmpty())
    {
        FDialogue_Item NextDialogue = DialogueData->GetNextDialogue();
        DisplayDialogue(NextDialogue);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue finished!"));
        RemoveFromParent(); // Hide UI when dialogue ends
    }
}
