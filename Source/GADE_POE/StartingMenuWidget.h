#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StartingMenuWidget.generated.h"

UCLASS()
class GADE_POE_API UStartingMenuWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    void OnStartClicked();

    UFUNCTION(BlueprintCallable)
    void OnExitClicked();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UUserWidget> MainMenuWidgetClass;

    UPROPERTY(Transient, meta = (BindWidgetAnim))
    UWidgetAnimation* FadeInAnimation;

    virtual void NativeConstruct() override;

private:
    void SwitchToMainMenu();
};
