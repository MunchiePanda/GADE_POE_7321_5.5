#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RacerTypes.h"
#include "AIRacer.h"
#include "AIRacerFactoryBase.generated.h"

UCLASS(Abstract)
class GADE_POE_API UAIRacerFactoryBase : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Racer Factory")
    virtual AAIRacer* CreateRacer(UWorld* World, ERacerType RacerType, const FVector& SpawnLocation, const FRotator& SpawnRotation) PURE_VIRTUAL(UAIRacerFactoryBase::CreateRacer, return nullptr;);

    UFUNCTION(BlueprintCallable, Category = "Racer Factory")
    virtual void SpawnRacers(UWorld* World, int32 MaxRacers, float FastChance, float MediumChance, float SlowChance, const FVector& BaseSpawnLocation, const FRotator& SpawnRotation) PURE_VIRTUAL(UAIRacerFactoryBase::SpawnRacers, );

    // Renamed from SpawnRacers(UWorld*) to SpawnRacersWithDefaults(UWorld*)
    UFUNCTION(BlueprintCallable, Category = "Racer Factory")
    virtual void SpawnRacersWithDefaults(UWorld* World) PURE_VIRTUAL(UAIRacerFactoryBase::SpawnRacersWithDefaults, );
};