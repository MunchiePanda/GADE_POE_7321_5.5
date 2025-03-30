#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CheckpointManager.h"
#include "Blueprint/UserWidget.h"
#include "RaceEndWidget.h"
#include "CheckpointRace_GMB.generated.h"

UCLASS()
class GADE_POE_API ACheckpointRace_GMB : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACheckpointRace_GMB();

	virtual void BeginPlay() override;
	UFUNCTION()
	void CheckRaceStatus(); // Function to check if race is over


protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD")
	TSubclassOf<UUserWidget> RaceHUDWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD")
	TSubclassOf<UUserWidget> TutorialWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD")
	TSubclassOf<UUserWidget> RaceEndWidgetClass;

private:
	UPROPERTY()
	UUserWidget* RaceHUDWidget;

	ACheckpointManager* CheckpointManager;

	URaceEndWidget* RaceEndWidget;
	void ShowTutorial();

};
