#pragma once

#include "CoreMinimal.h"
#include "AIRacerFactoryBase.h"
#include "AIRacerFactory.generated.h"

UCLASS(Blueprintable)
class GADE_POE_API UAIRacerFactory : public UAIRacerFactoryBase
{
    GENERATED_BODY()

public:
    UAIRacerFactory();

    virtual AAIRacer* CreateRacer(UWorld* World, ERacerType RacerType, const FVector& SpawnLocation, const FRotator& SpawnRotation) override;

    virtual void SpawnRacers(UWorld* World, int32 MaxRacers, float FastChance, float MediumChance, float SlowChance, const FVector& BaseSpawnLocation, const FRotator& SpawnRotation) override;

    virtual void SpawnRacersWithDefaults(UWorld* World) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer Factory")
    int32 MaxRacers = 9;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer Factory")
    float FastChance = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer Factory")
    float MediumChance = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer Factory")
    float SlowChance = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer Factory")
    FVector BaseSpawnLocation = FVector(31000.0f, -65000.0f, 50.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer Factory")
    FRotator SpawnRotation = FRotator(0.0f, 0.0f, 0.0f);

    // Moved to public non-UPROPERTY for internal use
    TSubclassOf<class AAIRacer> FastRacerClass;
    TSubclassOf<class AAIRacer> MediumRacerClass;
    TSubclassOf<class AAIRacer> SlowRacerClass;

private:
    TArray<AAIRacer*> SpawnedRacers;
};