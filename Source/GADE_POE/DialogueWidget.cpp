// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogueWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "TimerManager.h"
#include "Camera/CameraActor.h"

void UDialogueWidget::NativeConstruct()
{
    Super::NativeConstruct();
    // Find the Camera Actor in the level
    CameraActor = UGameplayStatics::GetActorOfClass(GetWorld(), ACameraActor::StaticClass()); 

	if (CameraActor) // Check if the camera actor exists
    {
        InitialCameraPosition = CameraActor->GetActorLocation(); // Store the initial camera position
        FinalCameraPosition = InitialCameraPosition + FVector(0, 0, -50); // Move camera slightly forward
    }

    // Get Player Controller
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PlayerController && CameraActor) // Check if the player controller and camera actor exist
    {
        PlayerController->SetViewTargetWithBlend(CameraActor, 1.0f); // 1-second smooth transition
    }

    // Bind button click event
    if (NextButton)
    {
        NextButton->OnClicked.AddDynamic(this, &UDialogueWidget::OnNextButtonClicked);
    }

    // Load dialogue data
	DialogueData = NewObject<UDialogue_Data>(); // Create a new dialogue data object
    if (DialogueData->LoadDialogue("GADE_TEST.json"))
    {
        OnNextButtonClicked();  // Show first dialogue automatically
    }
}

void UDialogueWidget::DisplayDialogue(FDialogue_Item DialogueItem)
{
    if (SpeakerNameText) // Check if the text block exists
    {
        SpeakerNameText->SetText(FText::FromString(DialogueItem.SpeakerName));
    }

	if (DialogueTextBlock) // Check if the text block exists
    {
		DialogueTextBlock->SetText(FText::FromString(DialogueItem.DialogueText)); // Set the dialogue text
        StartTypingEffect(DialogueItem.DialogueText); // Start typing
    }

    if (SpeakerPortraitImage)
    {
        // Load portrait (you'll need a texture asset path in Blueprint)
    }

    if (SpeakerAgeText) // Check if the text block exists
    {
        FString AgeText = FString::Printf(TEXT("Age: %d"), DialogueItem.Age);
        SpeakerAgeText->SetText(FText::FromString(AgeText));
    }
}


void UDialogueWidget::OnNextButtonClicked() // Handle the next button click
{
	if (DialogueData && !DialogueData->IsQueueEmpty()) // Check if there are more dialogues
    {
        FDialogue_Item NextDialogue = DialogueData->GetNextDialogue();
        DisplayDialogue(NextDialogue);

		// Move camera closer to the hamster
		MoveCameraCloser();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue finished!"));
        RemoveFromParent(); // Hide UI when dialogue ends
        LoadLevelAsync(FName("CheckpointMap")); // Load the next level
    }
}

void UDialogueWidget::MoveCameraCloser()    
{

	if (CameraActor) // Check if the camera actor exists
    {
        FVector CurrentPosition = CameraActor->GetActorLocation(); // Get the current camera position

        // Move towards the final position smoothly
        FVector NewPosition = FMath::VInterpTo(CurrentPosition, FinalCameraPosition, GetWorld()->GetDeltaSeconds(), CameraMoveSpeed);

		CameraActor->SetActorLocation(NewPosition); // Update the camera position
    }
}

void UDialogueWidget::ShowLoadingScreen()
{
	if (LoadingScreenWidgetClass) // Check if the class is set in the editor
    {
        LoadingScreenWidget = CreateWidget<UUserWidget>(GetWorld(), LoadingScreenWidgetClass);
		if (LoadingScreenWidget) // Check if the widget was created successfully
        {
            LoadingScreenWidget->AddToViewport(); // Add the widget to the viewport
            LoadingProgressSlider = Cast<USlider>(LoadingScreenWidget->GetWidgetFromName(TEXT("LoadingProgressSlider")));
        }
    }
}

void UDialogueWidget::HideLoadingScreen()
{
	if (LoadingScreenWidget) // Check if the widget exists
    {
        LoadingScreenWidget->RemoveFromParent(); // Remove the widget from the viewport
        LoadingScreenWidget = nullptr; // Set the widget pointer to nullptr
        LoadingProgressSlider = nullptr; // Set the slider pointer to nullptr
    }
}

void UDialogueWidget::LoadLevelAsync(const FName& LevelName) // Load the level
{
    if (LevelName.IsNone()) // Check if the level name is empty
    {
        UE_LOG(LogTemp, Error, TEXT("LoadLevelAsync called with an EMPTY level name!"));
        return;
    }

    ShowLoadingScreen(); // Show the loading screen

    // Delay opening the level by 2 seconds (simulating async load)
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, LevelName]()
        {
            HideLoadingScreen();
            UGameplayStatics::OpenLevel(this, LevelName);
        }, 2.0f, false);  // Adjust delay time if needed

	UE_LOG(LogTemp, Warning, TEXT("Starting async load for level: %s"), *LevelName.ToString()); // Log the level name
}


void UDialogueWidget::UpdateLoadingProgress(float Progress)
{
    if (LoadingProgressSlider) // Check if the slider exists
    {
        LoadingProgressSlider->SetValue(Progress); // Update the slider value
    }
}

void UDialogueWidget::StartTypingEffect(FString FullText)
{
    TypedText = TEXT(""); // Clear previous text
    CurrentCharIndex = 0; // Reset character index
    TargetText = FullText; // Set target text
    GetWorld()->GetTimerManager().SetTimer(TypingTimer, this, &UDialogueWidget::TypeNextLetter, 0.05f, true); // Start typing
}

void UDialogueWidget::TypeNextLetter()
{
    if (CurrentCharIndex < TargetText.Len()) // Check if there are more characters to type
    {
        TypedText.AppendChar(TargetText[CurrentCharIndex]); // Append the next character
        DialogueTextBlock->SetText(FText::FromString(TypedText)); // Update the text
        CurrentCharIndex++; // Move to the next character
    }
    else
    {
        GetWorld()->GetTimerManager().ClearTimer(TypingTimer); // Clear the timer
    }
}
