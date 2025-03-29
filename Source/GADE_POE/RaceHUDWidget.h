#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHamster.h"
#include "RaceHUDWidget.generated.h"

UCLASS()
class GADE_POE_API URaceHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    /** Gets the current speed (bind this to UI) */
    UFUNCTION(BlueprintCallable, Category = "HUD")
    FText GetSpeedText() const;

    /** Gets the elapsed time (bind this to UI) */
    UFUNCTION(BlueprintCallable, Category = "HUD")
    FText GetTimeText() const;

    /** Gets the lap count (bind this to UI) */
    UFUNCTION(BlueprintCallable, Category = "HUD")
    FText GetLapText() const;


    /** Updates the race stats */
    void SetRaceStats(float Speed, float TimeElapsed, int32 CurrentLap, int32 TotalLaps);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player")
    TSubclassOf<APlayerHamster> PlayerHamsterClass;

protected:
    /** Holds race data */
    float CurrentSpeed;
    float ElapsedTime;
    int32 LapNumber;
    int32 TotalLapCount;
};
