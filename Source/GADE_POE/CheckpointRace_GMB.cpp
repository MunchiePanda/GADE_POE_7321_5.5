#include "CheckpointRace_GMB.h"
#include "Kismet/GameplayStatics.h"
#include "RaceHUDWidget.h"      
#include "Blueprint/UserWidget.h" // Needed for CreateWidget()
#include "Engine/World.h"         // Needed for GetWorld()
ACheckpointRace_GMB::ACheckpointRace_GMB()
{
}

void ACheckpointRace_GMB::BeginPlay()
{
    Super::BeginPlay();

    if (RaceHUDWidgetClass)
    {
        URaceHUDWidget* RaceHUD = CreateWidget<URaceHUDWidget>(GetWorld(), RaceHUDWidgetClass);
        if (RaceHUD)
        {
            RaceHUD->AddToViewport();
        }
    }

    DialogueData = NewObject<UDialogue_Data>(this);

    DialogueData = NewObject<UDialogue_Data>(this);

    if (DialogueData)
    {
        bool bLoaded = DialogueData->LoadDialogue(TEXT("GADE_TEST.json"));
        if (bLoaded)
        {
            TArray<FDialogue_Item> DialogueItems;
            FDialogue_Item DialogueItem;
            while (DialogueData->GetNextDialogue().DialogueText != "")
            {
                DialogueItems.Add(DialogueItem);
            }

            if (DialogueItems.Num() > 0)  // Ensure there is dialogue data before accessing
            {
                for (const FDialogue_Item& Item : DialogueItems)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Speaker: %s, Text: %s"), *Item.SpeakerName, *Item.DialogueText);
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Dialogue data is empty!"));
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueData is null"));
    }
}
