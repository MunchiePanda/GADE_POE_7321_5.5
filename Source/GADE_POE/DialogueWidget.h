// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Dialogue_Data.h"
#include "Components/Slider.h"
#include "DialogueWidget.generated.h"

/**
 * 
 */
UCLASS()
class GADE_POE_API UDialogueWidget : public UUserWidget
{
	GENERATED_BODY()
	

public:
	virtual void NativeConstruct() override;

	/** Displays the dialogue */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void DisplayDialogue(FDialogue_Item DialogueItem);

    /** Handles Next Button Click */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void OnNextButtonClicked();

protected:
    /** Text Block for Speaker Name */
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* SpeakerNameText;

    /** Text Block for Dialogue Text */
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* DialogueTextBlock;

	/** Text Block for Speaker Age */
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* SpeakerAgeText;

    /** Image for Speaker Portrait */
    UPROPERTY(meta = (BindWidget))
    class UImage* SpeakerPortraitImage;

    /** Button to progress dialogue */
    UPROPERTY(meta = (BindWidget))
    class UButton* NextButton;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UUserWidget> LoadingScreenWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    AActor* CameraActor;

    FVector InitialCameraPosition;
    FVector FinalCameraPosition;

    float CameraMoveSpeed = 10.0f; // Adjust as needed

    void MoveCameraCloser();

private:
    /** Reference to Dialogue Data */
    UPROPERTY()
    UDialogue_Data* DialogueData;


    UPROPERTY()
    UUserWidget* LoadingScreenWidget; // Widget for loading screen

    UPROPERTY()
	USlider* LoadingProgressSlider; // Slider for loading progress

    FString TargetText; // Text to be typed
    FString TypedText; // Typed text
    int32 CurrentCharIndex; // Current character index
    FTimerHandle TypingTimer; // Timer for typing

	void ShowLoadingScreen(); // Show the loading screen
    void HideLoadingScreen(); // Hide the loading screen
    void LoadLevelAsync(const FName& LevelName); // Load a level asynchronously
    void UpdateLoadingProgress(float Progress); // Update the loading progress

    void StartTypingEffect(FString FullText); // Start the typing effect

	void TypeNextLetter(); // Type the next letter

};
