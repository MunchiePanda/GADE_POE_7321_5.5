// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "RaceGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class GADE_POE_API URaceGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite, Category = "Dialogue")
	FName TargetRaceLevel;
};
