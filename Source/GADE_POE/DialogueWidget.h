#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Dialogue_Data.h"
#include "Components/Slider.h"
#include "DialogueWidget.generated.h"

UCLASS()
class GADE_POE_API UDialogueWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void DisplayDialogue(FDialogue_Item DialogueItem);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void OnNextButtonClicked();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FName TargetRaceLevel;

protected:
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* SpeakerNameText;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* DialogueTextBlock;

    UPROPERTY(meta = (BindWidget))
    class UImage* SpeakerPortraitImage;

    UPROPERTY(meta = (BindWidget))
    class UButton* NextButton;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* AgeText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UUserWidget> LoadingScreenWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    AActor* CameraActor;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    TMap<FString, UTexture2D*> SpeakerPortraits;

    FVector InitialCameraPosition;
    FVector FinalCameraPosition;
    float CameraMoveSpeed = 10.0f;

    void MoveCameraCloser();

private:
    UPROPERTY()
    UDialogue_Data* DialogueData;

    UPROPERTY()
    UUserWidget* LoadingScreenWidget;

    UPROPERTY()
    USlider* LoadingProgressSlider;

    void ShowLoadingScreen();
    void HideLoadingScreen();
    void LoadLevelAsync(const FName& LevelName);
    void UpdateLoadingProgress(float Progress);

    FString CurrentText;
    FString DisplayedText;
    int32 CurrentCharIndex;
    FTimerHandle TypingTimerHandle;

    void StartTypingEffect(const FString& FullText);
    void TypeNextLetter();
};