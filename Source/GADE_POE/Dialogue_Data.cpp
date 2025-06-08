#include "Dialogue_Data.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "JsonObjectConverter.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

bool UDialogue_Data::LoadDialogue(const FString& FileName)
{
    DialogueQueue.Clear();  // Clear queue before loading new data

    FString FilePath = FPaths::ProjectDir() + "Content/JSONFILES/" + FileName;
    UE_LOG(LogTemp, Warning, TEXT("Dialogue_Data: Full file path: %s"), *FilePath);

    // Check if file exists
    if (!FPaths::FileExists(FilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Dialogue_Data: File does not exist at path: %s"), *FilePath);
        return false;
    }

    FString JsonString;
    if (FFileHelper::LoadFileToString(JsonString, *FilePath))
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue_Data: Successfully loaded JSON file with length: %d"), JsonString.Len());

        TArray<TSharedPtr<FJsonValue>> JsonArray;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

        if (FJsonSerializer::Deserialize(Reader, JsonArray))
        {
            UE_LOG(LogTemp, Warning, TEXT("Dialogue_Data: Successfully parsed JSON array with %d entries"), JsonArray.Num());

            for (const TSharedPtr<FJsonValue>& JsonValue : JsonArray)
            {
                if (!JsonValue.IsValid() || !JsonValue->AsObject().IsValid())
                {
                    UE_LOG(LogTemp, Error, TEXT("Dialogue_Data: Invalid JSON entry detected! Skipping..."));
                    continue;
                }

                FDialogue_Item DialogueItem;
                if (FJsonObjectConverter::JsonObjectToUStruct(JsonValue->AsObject().ToSharedRef(), &DialogueItem))
                {
                    DialogueQueue.Enqueue(DialogueItem);
                    UE_LOG(LogTemp, Warning, TEXT("Dialogue_Data: Successfully loaded dialogue: %s - %s"),
                        *DialogueItem.Name, *DialogueItem.DialogueText);
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("Dialogue_Data: Failed to convert JSON to FDialogue_Item!"));
                }
            }
            return true;
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Dialogue_Data: Failed to parse JSON as an array. JSON content: %s"), *JsonString);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Dialogue_Data: Failed to load file: %s"), *FilePath);
    }

    return false;
}



FDialogue_Item UDialogue_Data::GetNextDialogue()
{
    UE_LOG(LogTemp, Warning, TEXT("Queue size BEFORE dequeue: %d"), DialogueQueue.GetSize());

    FDialogue_Item NextDialogue;
    if (DialogueQueue.Dequeue(NextDialogue))
    {
        UE_LOG(LogTemp, Warning, TEXT("Dequeued: %s - %s"), *NextDialogue.Name, *NextDialogue.DialogueText);
        return NextDialogue;
    }

    UE_LOG(LogTemp, Warning, TEXT("Dialogue queue is empty! Returning default dialogue."));
    return FDialogue_Item(); // Return an empty struct if queue is empty
}



bool UDialogue_Data::IsQueueEmpty() const
{
    return DialogueQueue.IsEmpty();
}
