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
    ABeginnerRace_GMB();

    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spectators")
    TArray<ASpectator*> Spectators;
};