/**
 AIRacer Class
 
 This class represents an AI-controlled racing character in the game.
 It handles movement, physics, racing behavior, and navigation on the race track.
 The class extends Character to utilize Unreal's character movement system.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RacerTypes.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "AIRacer.generated.h"

// Forward declarations
class AAIRacerContoller;
class ABeginnerRaceGameState;

UCLASS()
class GADE_POE_API AAIRacer : public ACharacter
{
    GENERATED_BODY()

public:
    /** Constructor - Sets up default values and component initialization */
    AAIRacer();

    /** Called when the game starts or when spawned */
    virtual void BeginPlay() override;
    
    /** Called every frame to update the racer's state */
    virtual void Tick(float DeltaTime) override;
    
    /** Initializes the racer's attributes based on its type (Fast, Medium, Slow) */
    void SetupRacerAttributes();

    /** Reference to the game state for race management */
    UPROPERTY()
    ABeginnerRaceGameState* GameState;

    /** Defines the type of racer (Fast, Medium, Slow) affecting its performance characteristics */
    UPROPERTY(EditAnywhere, Category = "Racer")
    ERacerType RacerType;

    /** Maximum speed the racer can achieve (units/second) */
    UPROPERTY(EditAnywhere, Category = "Racer")
    float MaxSpeed;

    /** Maximum acceleration rate (units/second²) */
    UPROPERTY(EditAnywhere, Category = "Racer")
    float MaxAcceleration;

    /** Skeletal mesh component for the racer's visual representation */
    UPROPERTY(EditAnywhere, Category = "Racer")
    USkeletalMeshComponent* RacerMesh;

    /** Physics body component for collision and physics simulation */
    UPROPERTY(EditAnywhere, Category = "Racer")
    UStaticMeshComponent* PhysicsBody;

    /** Current lap number in the race */
    UPROPERTY(VisibleAnywhere, Category = "Race")
    int32 LapCount;

    /** Number of waypoints passed in the current lap */
    UPROPERTY(VisibleAnywhere, Category = "Race")
    int32 WaypointsPassed;

    /** Current speed of the racer */
    UPROPERTY()
    float CurrentSpeed = 0.0f;

    /** Distance at which the racer starts braking before corners (units) */
    UPROPERTY(EditAnywhere, Category = "Racing")
    float BrakingDistance = 500.0f;

    /** Speed multiplier applied when cornering (percentage) */
    UPROPERTY(EditAnywhere, Category = "Racing")
    float CorneringSpeedMultiplier = 0.7f;

    /** Rate at which the racer accelerates (units/second²) */
    UPROPERTY(EditAnywhere, Category = "Racing")
    float AccelerationRate = 2000.0f;

    /** Rate at which the racer decelerates when braking (units/second²) */
    UPROPERTY(EditAnywhere, Category = "Racing")
    float BrakingRate = 4000.0f;

    /** Minimum speed maintained during cornering (units/second) */
    UPROPERTY(EditAnywhere, Category = "Racing")
    float MinCorneringSpeed = 300.0f;

    /** Maximum angle at which full cornering behavior is applied (degrees) */
    UPROPERTY(EditAnywhere, Category = "Racing")
    float MaxCorneringAngle = 60.0f;

protected:
    /** Updates the racer's movement behavior each frame */
    void UpdateRacingBehavior(float DeltaTime);
    
    /** Adjusts the racer's speed based on corner angle */
    void AdjustSpeedForCorner(float CornerAngle);
    
    /** Calculates the desired speed based on distance to corner and corner angle */
    float CalculateDesiredSpeed(float DistanceToCorner, float CornerAngle);
};