#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RaceHUDWidget.generated.h"

UCLASS()
class GADE_POE_API URaceHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Updates the speed display */
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void UpdateSpeed(float Speed);

    /** Updates the time display */
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void UpdateTime(float TimeElapsed);

    /** Updates the lap count display */
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void UpdateLaps(int32 CurrentLap, int32 TotalLaps);

protected:
    /** Speed text UI element */
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* SpeedText;

    /** Time text UI element */
    UPROPERTY(meta = (BindWidget))
    UTextBlock* TimeText;

    /** Lap count text UI element */
    UPROPERTY(meta = (BindWidget))
    UTextBlock* LapText;
};
