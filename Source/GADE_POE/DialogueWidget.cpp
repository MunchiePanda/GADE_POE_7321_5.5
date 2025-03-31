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

    if (CameraActor)
    {
        InitialCameraPosition = CameraActor->GetActorLocation();
        FinalCameraPosition = InitialCameraPosition + FVector(0, 0, -50); // Move camera slightly forward
    }

    // Get Player Controller
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PlayerController && CameraActor)
    {
        PlayerController->SetViewTargetWithBlend(CameraActor, 1.0f); // 1-second smooth transition
    }

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

    if (CameraActor)
    {
        FVector CurrentPosition = CameraActor->GetActorLocation();

        // Move towards the final position smoothly
        FVector NewPosition = FMath::VInterpTo(CurrentPosition, FinalCameraPosition, GetWorld()->GetDeltaSeconds(), CameraMoveSpeed);

        CameraActor->SetActorLocation(NewPosition);
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

void UDialogueWidget::LoadLevelAsync(const FName& LevelName)
{
    if (LevelName.IsNone())
    {
        UE_LOG(LogTemp, Error, TEXT("LoadLevelAsync called with an EMPTY level name!"));
        return;
    }

    ShowLoadingScreen();

    // Delay opening the level by 2 seconds (simulating async load)
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, LevelName]()
        {
            HideLoadingScreen();
            UGameplayStatics::OpenLevel(this, LevelName);
        }, 2.0f, false);  // Adjust delay time if needed

    UE_LOG(LogTemp, Warning, TEXT("Starting async load for level: %s"), *LevelName.ToString());
}


void UDialogueWidget::UpdateLoadingProgress(float Progress)
{
    if (LoadingProgressSlider) // Check if the slider exists
    {
        LoadingProgressSlider->SetValue(Progress); // Update the slider value
    }
}
