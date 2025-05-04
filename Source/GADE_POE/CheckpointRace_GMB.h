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
	TSubclassOf<UUserWidget> RaceHUDWidgetClass; // Class for the race HUD

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD")
	TSubclassOf<UUserWidget> TutorialWidgetClass; // Class for the tutorial

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD")
	TSubclassOf<UUserWidget> RaceEndWidgetClass; // Class for the race end

private:
	UPROPERTY() // Property for the race HUD
	UUserWidget* RaceHUDWidget;

	ACheckpointManager* CheckpointManager; // Reference to the checkpoint manager

	URaceEndWidget* RaceEndWidget; // Reference to the race end widget
	void ShowTutorial(); // Function to show the tutorial

};
