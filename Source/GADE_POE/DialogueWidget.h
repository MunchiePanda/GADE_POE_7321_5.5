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

    /** Image for Speaker Portrait */
    UPROPERTY(meta = (BindWidget))
    class UImage* SpeakerPortraitImage;

    /** Button to progress dialogue */
    UPROPERTY(meta = (BindWidget))
    class UButton* NextButton;

	/** Button to skip dialogue */
	UPROPERTY(meta = (BindWidget))
    class UTextBlock* AgeText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI") // Reference to the loading screen widget class
    TSubclassOf<UUserWidget> LoadingScreenWidgetClass; 

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera") // Reference to the camera actor
    AActor* CameraActor;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	TMap<FString, UTexture2D*> SpeakerPortraits; // Map to store speaker portraits

	FVector InitialCameraPosition; // Initial camera position
    FVector FinalCameraPosition;    // Final camera position

    float CameraMoveSpeed = 10.0f; // Adjust as needed

    void MoveCameraCloser();

private:
    /** Reference to Dialogue Data */
    UPROPERTY()
    UDialogue_Data* DialogueData;


	UPROPERTY() // Reference to the loading screen widget
    UUserWidget* LoadingScreenWidget;

	UPROPERTY() // Reference to the loading progress slider
    USlider* LoadingProgressSlider;

	void ShowLoadingScreen(); // Show the loading screen
    void HideLoadingScreen(); // Hide the loading screen
    void LoadLevelAsync(const FName& LevelName); // Load a level asynchronously
    void UpdateLoadingProgress(float Progress); // Update the loading progress

    /** Typing Effects */
    FString CurrentText;
    FString DisplayedText;
    int32 CurrentCharIndex;
    FTimerHandle TypingTimerHandle;

    void StartTypingEffect(const FString& FullText);
    void TypeNextLetter();

};
