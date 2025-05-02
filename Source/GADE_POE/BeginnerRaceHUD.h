
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BiginnerRaceGameState.h"
#include "BeginnerRaceHUD.generated.h"


class APlayerHamster;
/**
 * 
 */
UCLASS()
class GADE_POE_API UBeginnerRaceHUD : public UUserWidget
{
	GENERATED_BODY()
public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* LapCounter;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* PositionDisplay;

    UPROPERTY()
    APlayerHamster* PlayerHamster;

    UPROPERTY()
    ABeginnerRaceGameState* GameState;

    void UpdateLapCounter();
    void UpdatePositionDisplay();
};
