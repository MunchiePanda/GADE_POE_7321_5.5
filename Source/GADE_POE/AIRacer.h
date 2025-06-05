#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RacerTypes.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "AIRacer.generated.h"

class AAIRacerContoller;
class ABeginnerRaceGameState;

UCLASS()
class GADE_POE_API AAIRacer : public ACharacter
{
    GENERATED_BODY()

public:
    AAIRacer();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    void SetupRacerAttributes();

    UPROPERTY()
    ABeginnerRaceGameState* GameState;

    UPROPERTY(EditAnywhere, Category = "Racer")
    ERacerType RacerType;

    UPROPERTY(EditAnywhere, Category = "Racer")
    float MaxSpeed;

    UPROPERTY(EditAnywhere, Category = "Racer")
    float MaxAcceleration;

    UPROPERTY(EditAnywhere, Category = "Racer")
    USkeletalMeshComponent* RacerMesh;

    UPROPERTY(EditAnywhere, Category = "Racer")
    UStaticMeshComponent* PhysicsBody;

    UPROPERTY(VisibleAnywhere, Category = "Race")
    int32 LapCount;

    UPROPERTY(VisibleAnywhere, Category = "Race")
    int32 WaypointsPassed;

    UPROPERTY()
    float CurrentSpeed = 0.0f;
};