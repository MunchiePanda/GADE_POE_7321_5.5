#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Spectator.h"
#include "BeginnerRace_GMB.generated.h"

UCLASS()
class GADE_POE_API ABeginnerRace_GMB : public AGameModeBase
{
    GENERATED_BODY()

public:
	ABeginnerRace_GMB(); // Constructor

    virtual void BeginPlay() override; 

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spectators")
	TArray<ASpectator*> Spectators; // Array to hold all spectators in the level

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD")
    TSubclassOf<UUserWidget> TutorialWidgetClass; // The class of the tutorial widget

protected:
	void ShowTutorial(); // Function to show the tutorial UI
};