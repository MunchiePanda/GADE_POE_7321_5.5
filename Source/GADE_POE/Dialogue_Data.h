#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include "DIalogueQueueTemp.h"
#include "Dialogue_Data.generated.h"

/**
 * Structure to hold dialogue data.
 */
USTRUCT(BlueprintType)
struct FDialogue_Item : public FTableRowBase
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dialogue")
    FString Name; // <-- Add this to match JSON

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dialogue")
    FString SpeakerName; // The name of the speaker

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dialogue")
    FString SpeakerPortrait; // The portrait of the speaker

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dialogue")
    FString DialogueText; // The dialogue text

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dialogue")
    int32 Age; // The age of person

    FDialogue_Item() : Name(TEXT("")), SpeakerName(TEXT("")), SpeakerPortrait(TEXT("")), DialogueText(TEXT("")), Age(0) {}

    /** Constructor for quick initialization */
    FDialogue_Item(const FString& InName, const FString& InSpeaker, const FString& InPortrait, const FString& InText, int32 InAge)
        : Name(InName), SpeakerName(InSpeaker), SpeakerPortrait(InPortrait), DialogueText(InText), Age(InAge) {
    }
};

UCLASS()
class GADE_POE_API UDialogue_Data : public UObject
{
    GENERATED_BODY()

public:
    /** Loads the dialogue from a JSON file */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool LoadDialogue(const FString& FileName);

    /** Retrieves the next dialogue item */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FDialogue_Item GetNextDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsQueueEmpty() const;

private:

    DialogueQueueTemp<FDialogue_Item> DialogueQueue;
    /** Holds all dialogue items in an array */
    //TArray<FDialogue_Item> DialogueQueue;

    /** Keeps track of the current dialogue position */
    int32 CurrentIndex = 0;
};
