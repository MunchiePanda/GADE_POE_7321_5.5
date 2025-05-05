#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RacerTypes.h"
#include "AIRacer.h"
#include "AIRacerFactoryBase.generated.h"

UCLASS(Abstract) // Abstract class to prevent instantiation of this class
class GADE_POE_API AAIRacerFactoryBase : public AActor
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Racer Factory") // Pure virtual function
    virtual AAIRacer* CreateRacer(UWorld* World, ERacerType RacerType, const FVector& SpawnLocation, const FRotator& SpawnRotation) PURE_VIRTUAL(AAIRacerFactoryBase::CreateRacer, return nullptr;);

    UFUNCTION(BlueprintCallable, Category = "Racer Factory") // Pure virtual function
    virtual void SpawnRacers(UWorld* World, int32 MaxRacers, float FastChance, float MediumChance, float SlowChance, const FRotator& SpawnRotation) PURE_VIRTUAL(AAIRacerFactoryBase::SpawnRacers, );

    UFUNCTION(BlueprintCallable, Category = "Racer Factory") // Pure virtual function 
    virtual void SpawnRacersWithDefaults(UWorld* World) PURE_VIRTUAL(AAIRacerFactoryBase::SpawnRacersWithDefaults, );
};