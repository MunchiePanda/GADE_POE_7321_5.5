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
#include "RaceGameInstance.h"

void UDialogueWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Find the Camera Actor in the level
    CameraActor = UGameplayStatics::GetActorOfClass(GetWorld(), ACameraActor::StaticClass());
    if (CameraActor)
    {
        InitialCameraPosition = CameraActor->GetActorLocation();
        FinalCameraPosition = InitialCameraPosition + FVector(0, 0, -50);
    }

    // Get Player Controller
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PlayerController && CameraActor)
    {
        PlayerController->SetViewTargetWithBlend(CameraActor, 1.0f);
    }

    // Bind button click event
    if (NextButton)
    {
        NextButton->OnClicked.AddDynamic(this, &UDialogueWidget::OnNextButtonClicked);
    }

    // Get current level name
    FString LevelName = GetWorld()->GetMapName();
    UE_LOG(LogTemp, Log, TEXT("DialogueWidget: Current level: %s"), *LevelName);

    // Set JSON file and TargetRaceLevel based on level name
    FString JsonFile;
    if (LevelName.Contains("BeginnerDialogueLVL"))
    {
        JsonFile = "BeginnerRaceDialogue.json";
        TargetRaceLevel = FName("BeginnerMap");
    }
    else if (LevelName.Contains("CheckpointDialogueLVL"))
    {
        JsonFile = "GADE_TEST.json";
        TargetRaceLevel = FName("CheckpointMap");
    }
    else if (LevelName.Contains("AdvancedDialogueLVL"))
    {
        JsonFile = "AdvancedRaceDialogue.json";
		TargetRaceLevel = FName("AdvancedMap");
    }
    else
    {
        // Fallback to Game Instance if level name is unrecognized
        URaceGameInstance* GameInstance = Cast<URaceGameInstance>(GetGameInstance());
        if (GameInstance && !GameInstance->TargetRaceLevel.IsNone())
        {
            TargetRaceLevel = GameInstance->TargetRaceLevel;
            UE_LOG(LogTemp, Log, TEXT("DialogueWidget: Set TargetRaceLevel from GameInstance: %s"), *TargetRaceLevel.ToString());
            if (TargetRaceLevel == FName("BeginnerDialogueLVL"))
            {
                JsonFile = "BeginnerRaceDialogue.json";
                TargetRaceLevel = FName("BeginnerMap");
            }
            else if (TargetRaceLevel == FName("CheckpointDialogueLVL"))
            {
                JsonFile = "GADE_TEST.json";
                TargetRaceLevel = FName("CheckpointMap");
            }
			else if (TargetRaceLevel == FName("AdvancedDialogueLVL"))
			{
				JsonFile = "AdvancedRaceDialogue.json";
				TargetRaceLevel = FName("AdvancedMap");
			}
            else
            {
                UE_LOG(LogTemp, Error, TEXT("DialogueWidget: Invalid TargetRaceLevel %s from GameInstance, defaulting to GADE_TEST.json"), *TargetRaceLevel.ToString());
                JsonFile = "GADE_TEST.json";
                TargetRaceLevel = FName("CheckpointMap");
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("DialogueWidget: Unrecognized level %s and no valid GameInstance TargetRaceLevel, defaulting to GADE_TEST.json"), *LevelName);
            JsonFile = "GADE_TEST.json";
            TargetRaceLevel = FName("CheckpointMap");
        }
    }

    // Load dialogue data
    DialogueData = NewObject<UDialogue_Data>();
    if (DialogueData->LoadDialogue(JsonFile))
    {
        OnNextButtonClicked();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("DialogueWidget: Failed to load JSON file %s"), *JsonFile);
    }
}

void UDialogueWidget::NativeDestruct()
{
    // Clear the typing timer
    if (GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(TypingTimerHandle))
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueWidget: Clearing active TypingTimerHandle"));
        GetWorld()->GetTimerManager().ClearTimer(TypingTimerHandle);
    }

    // Remove loading screen widget
    if (LoadingScreenWidget)
    {
        LoadingScreenWidget->RemoveFromParent();
        LoadingScreenWidget = nullptr;
    }

    // Nullify pointers
    LoadingProgressSlider = nullptr;
    DialogueData = nullptr;

    // Unbind button delegate
    if (NextButton)
    {
        NextButton->OnClicked.RemoveDynamic(this, &UDialogueWidget::OnNextButtonClicked);
    }

    Super::NativeDestruct();
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
        StartTypingEffect(DialogueItem.DialogueText);
    }

    if (SpeakerPortraitImage && SpeakerPortraits.Contains(DialogueItem.SpeakerPortrait))
    {
        SpeakerPortraitImage->SetBrushFromTexture(SpeakerPortraits[DialogueItem.SpeakerPortrait]);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No portrait found for %s"), *DialogueItem.SpeakerPortrait);
    }

    if (AgeText)
    {
        AgeText->SetText(FText::FromString(FString::Printf(TEXT("Age: %d"), DialogueItem.Age)));
    }
}

void UDialogueWidget::OnNextButtonClicked()
{
    if (DialogueData && !DialogueData->IsQueueEmpty())
    {
        FDialogue_Item NextDialogue = DialogueData->GetNextDialogue();
        DisplayDialogue(NextDialogue);
        MoveCameraCloser();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue finished!"));
        RemoveFromParent();
        if (!TargetRaceLevel.IsNone())
        {
            LoadLevelAsync(TargetRaceLevel);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("DialogueWidget: TargetRaceLevel is not set, defaulting to CheckpointMap"));
            LoadLevelAsync(FName("CheckpointMap"));
        }
    }
}

void UDialogueWidget::MoveCameraCloser()
{
    if (CameraActor)
    {
        FVector CurrentPosition = CameraActor->GetActorLocation();
        FVector NewPosition = FMath::VInterpTo(CurrentPosition, FinalCameraPosition, GetWorld()->GetDeltaSeconds(), CameraMoveSpeed);
        CameraActor->SetActorLocation(NewPosition);
    }
}

void UDialogueWidget::ShowLoadingScreen()
{
    if (LoadingScreenWidgetClass)
    {
        LoadingScreenWidget = CreateWidget<UUserWidget>(GetWorld(), LoadingScreenWidgetClass);
        if (LoadingScreenWidget)
        {
            LoadingScreenWidget->AddToViewport();
            LoadingProgressSlider = Cast<USlider>(LoadingScreenWidget->GetWidgetFromName(TEXT("LoadingProgressSlider")));
        }
    }
}

void UDialogueWidget::HideLoadingScreen()
{
    if (LoadingScreenWidget)
    {
        LoadingScreenWidget->RemoveFromParent();
        LoadingScreenWidget = nullptr;
    }
    LoadingProgressSlider = nullptr;
}

void UDialogueWidget::LoadLevelAsync(const FName& LevelName)
{
    if (LevelName.IsNone())
    {
        UE_LOG(LogTemp, Error, TEXT("LoadLevelAsync called with an EMPTY level name!"));
        return;
    }

    ShowLoadingScreen();

    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, LevelName]()
        {
            HideLoadingScreen();
            UGameplayStatics::OpenLevel(this, LevelName);
        }, 2.0f, false);

    UE_LOG(LogTemp, Warning, TEXT("Starting async load for level: %s"), *LevelName.ToString());
}

void UDialogueWidget::UpdateLoadingProgress(float Progress)
{
    if (LoadingScreenWidget)
    {
        LoadingProgressSlider->SetValue(Progress);
    }
}

void UDialogueWidget::StartTypingEffect(const FString& FullText)
{
    CurrentText = FullText;
    DisplayedText = TEXT("");
    CurrentCharIndex = 0;

    GetWorld()->GetTimerManager().SetTimer(TypingTimerHandle, this, &UDialogueWidget::TypeNextLetter, 0.05f, true);
}

void UDialogueWidget::TypeNextLetter()
{
    if (CurrentCharIndex < CurrentText.Len())
    {
        DisplayedText.AppendChar(CurrentText[CurrentCharIndex]);
        if (DialogueTextBlock)
        {
            DialogueTextBlock->SetText(FText::FromString(DisplayedText));
        }
        CurrentCharIndex++;
    }
    else
    {
        GetWorld()->GetTimerManager().ClearTimer(TypingTimerHandle);
    }
}