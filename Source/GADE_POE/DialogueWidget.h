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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UUserWidget> LoadingScreenWidgetClass;

private:
    /** Reference to Dialogue Data */
    UPROPERTY()
    UDialogue_Data* DialogueData;


    UPROPERTY()
    UUserWidget* LoadingScreenWidget;

    UPROPERTY()
    USlider* LoadingProgressSlider;

	void ShowLoadingScreen(); // Show the loading screen
    void HideLoadingScreen(); // Hide the loading screen
    void LoadLevelAsync(const FName& LevelName); // Load a level asynchronously
    void UpdateLoadingProgress(float Progress); // Update the loading progress

};
