/**
 Factory for creating and managing AI racers
 
 Handles spawning different types of racers with configurable probabilities
 and manages their spawn locations through spawn points.
 */

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
    /** Constructor - Sets up default factory values */
    AAIRacerFactory();

protected:
    /** Called when the game starts */
    virtual void BeginPlay() override;

public:
    /**
     * Creates a single AI racer of specified type at given location
     * @param World - Current world context
     * @param RacerType - Type of racer to create (Fast, Medium, Slow)
     * @param SpawnLocation - Location to spawn the racer
     * @param SpawnRotation - Initial rotation of the racer
     * @return Pointer to the created AIRacer, or nullptr if creation fails
     */
    virtual AAIRacer* CreateRacer(UWorld* World, ERacerType RacerType, const FVector& SpawnLocation, const FRotator& SpawnRotation) override;

    /**
     * Spawns multiple racers with specified probabilities
     * @param World - Current world context
     * @param MaxRacers - Maximum number of racers to spawn
     * @param FastChance - Probability of spawning a fast racer
     * @param MediumChance - Probability of spawning a medium racer
     * @param SlowChance - Probability of spawning a slow racer
     * @param SpawnRotation - Initial rotation for spawned racers
     */
    virtual void SpawnRacers(UWorld* World, int32 MaxRacers, float FastChance, float MediumChance, float SlowChance, const FRotator& SpawnRotation) override;

    /** Spawns racers using the factory's default configuration values */
    virtual void SpawnRacersWithDefaults(UWorld* World) override;

    /** Maximum number of racers that can be spawned */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer Factory")
    int32 MaxRacers = 9;

    /** Probability (0-1) of spawning a fast racer */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer Factory")
    float FastChance = 0.2f;  // 20% chance

    /** Probability (0-1) of spawning a medium racer */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer Factory")
    float MediumChance = 0.5f;  // 50% chance

    /** Probability (0-1) of spawning a slow racer */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer Factory")
    float SlowChance = 0.3f;  // 30% chance

    /** Default rotation applied to newly spawned racers */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer Factory")
    FRotator SpawnRotation = FRotator(0.0f, 0.0f, 0.0f);

    /** Blueprint class for fast racers */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer Factory")
    TSubclassOf<class AAIRacer> FastRacerClass;

    /** Blueprint class for medium racers */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer Factory")
    TSubclassOf<class AAIRacer> MediumRacerClass;

    /** Blueprint class for slow racers */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer Factory")
    TSubclassOf<class AAIRacer> SlowRacerClass;

    /** Blueprint class for spawn point actors */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer Factory")
    TSubclassOf<ARacerSpawnPoint> SpawnPointClass;

private:
    /** Keeps track of all racers created by this factory */
    TArray<AAIRacer*> SpawnedRacers;
    
    /** Available spawn points for placing new racers */
    TArray<ARacerSpawnPoint*> SpawnPoints;
};