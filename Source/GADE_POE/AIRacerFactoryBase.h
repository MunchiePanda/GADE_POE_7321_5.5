#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RacerTypes.h"
#include "AIRacer.h"
#include "AIRacerFactoryBase.generated.h"

UCLASS(Abstract)
class GADE_POE_API AAIRacerFactoryBase : public AActor
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Racer Factory")
    virtual AAIRacer* CreateRacer(UWorld* World, ERacerType RacerType, const FVector& SpawnLocation, const FRotator& SpawnRotation) PURE_VIRTUAL(AAIRacerFactoryBase::CreateRacer, return nullptr;);

    UFUNCTION(BlueprintCallable, Category = "Racer Factory")
    virtual void SpawnRacers(UWorld* World, int32 MaxRacers, float FastChance, float MediumChance, float SlowChance, const FRotator& SpawnRotation) PURE_VIRTUAL(AAIRacerFactoryBase::SpawnRacers, );

    UFUNCTION(BlueprintCallable, Category = "Racer Factory")
    virtual void SpawnRacersWithDefaults(UWorld* World) PURE_VIRTUAL(AAIRacerFactoryBase::SpawnRacersWithDefaults, );
};