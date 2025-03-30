#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CheckpointRace_GMB.generated.h"

UCLASS()
class GADE_POE_API ACheckpointRace_GMB : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACheckpointRace_GMB();

	virtual void BeginPlay() override;


protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD")
	TSubclassOf<UUserWidget> RaceHUDWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD")
	TSubclassOf<UUserWidget> TutorialWidgetClass;

private:
	UPROPERTY()
	UUserWidget* RaceHUDWidget;

	void ShowTutorial();

};
