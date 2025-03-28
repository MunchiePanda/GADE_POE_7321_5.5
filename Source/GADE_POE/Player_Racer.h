#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ChaosVehicleMovementComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Player_Racer.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UChaosVehicleMovementComponent;

UCLASS()
class GADE_POE_API APlayer_Racer : public ACharacter
{
    GENERATED_BODY()

public:
    APlayer_Racer();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Vehicle Controls
    void ThrottleInput(float Val);
    void SteerInput(float Val);
    void HandbrakePressed();
    void HandbrakeReleased();

    // Camera Controls
    void LookUp(float Val);
    void Turn(float Val);

private:
    UPROPERTY(VisibleAnywhere, Category = "Camera")
    USpringArmComponent* SpringArm;

    UPROPERTY(VisibleAnywhere, Category = "Camera")
    UCameraComponent* Camera;

    UPROPERTY(VisibleAnywhere, Category = "Vehicle")
    UChaosWheeledVehicleMovementComponent* VehicleMovement;

    // Movement Properties
    UPROPERTY(EditAnywhere, Category = "Vehicle Settings")
    float MaxThrottle;

    UPROPERTY(EditAnywhere, Category = "Vehicle Settings")
    float MaxSteerAngle;
};
