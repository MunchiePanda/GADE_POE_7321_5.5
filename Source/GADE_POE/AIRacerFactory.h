#pragma once

#include "CoreMinimal.h"
#include "AIRacerFactoryBase.h"
#include "RacerSpawnPoint.h"
#include "AIRacerFactory.generated.h"

UCLASS(Blueprintable)
class GADE_POE_API AAIRacerFactory : public AAIRacerFactoryBase
{
    GENERATED_BODY()

public:
    AAIRacerFactory();

protected:
    virtual void BeginPlay() override;

public:
    virtual AAIRacer* CreateRacer(UWorld* World, ERacerType RacerType, const FVector& SpawnLocation, const FRotator& SpawnRotation) override; //create a racer

    virtual void SpawnRacers(UWorld* World, int32 MaxRacers, float FastChance, float MediumChance, float SlowChance, const FRotator& SpawnRotation) override; //spawn racers

    virtual void SpawnRacersWithDefaults(UWorld* World) override; //spawn racers with default values

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer Factory")
    int32 MaxRacers = 9; // Max number of racers

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer Factory")
    float FastChance = 0.2f; // Chance of spawning a fast racer

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer Factory")
    float MediumChance = 0.5f; // Chance of spawning a medium racer

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer Factory")
    float SlowChance = 0.3f; // Chance of spawning a slow racer

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer Factory")
    FRotator SpawnRotation = FRotator(0.0f, 0.0f, 0.0f); // Spawn rotation

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer Factory")
    TSubclassOf<class AAIRacer> FastRacerClass; // Class of the fast racer

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer Factory")
    TSubclassOf<class AAIRacer> MediumRacerClass; // Class of the medium racer

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer Factory")
    TSubclassOf<class AAIRacer> SlowRacerClass; // Class of the slow racer

   UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer Factory")
    TSubclassOf<ARacerSpawnPoint> SpawnPointClass; // Class of the spawn point actor

private:
    TArray<AAIRacer*> SpawnedRacers; // Array of spawned racers
    TArray<ARacerSpawnPoint*> SpawnPoints; // Array of spawn points 
};