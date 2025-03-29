#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHamster.h"
#include "CheckpointManager.h"
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

	UFUNCTION(BlueprintCallable, Category = "HUD")
    FText GetCheckpointText() const;


    /** Updates the race stats */
    void SetRaceStats(float Speed, float TimeElapsed, int32 CurrentLap, int32 TotalLaps);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player")
    TSubclassOf<APlayerHamster> PlayerHamsterClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player")
   ACheckpointManager* CheckpointManagerClass;

protected:
    /** Holds race data */
    float CurrentSpeed;
    float ElapsedTime;
    int32 LapNumber;
    int32 TotalLapCount;
};
