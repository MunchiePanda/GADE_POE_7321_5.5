// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AIRacerFactory.h"
#include "AIRacer.h"
#include "BeginnerRace_GMB.generated.h"

UCLASS()
class GADE_POE_API ABeginnerRace_GMB : public AGameModeBase
{
    GENERATED_BODY()

public:
    ABeginnerRace_GMB();

    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer Factory")
    UAIRacerFactory* RacerFactory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer Factory")
    TSubclassOf<class AAIRacer> FastRacerClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer Factory")
    TSubclassOf<class AAIRacer> MediumRacerClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer Factory")
    TSubclassOf<class AAIRacer> SlowRacerClass;
};