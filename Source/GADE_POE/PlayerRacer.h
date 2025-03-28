#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerRacer.generated.h"

class USpringArmComponent;
class UCameraComponent;

UCLASS()
class GADE_POE_API APlayerRacer : public ACharacter
{
    GENERATED_BODY()

public:
    APlayerRacer();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
    /** Vehicle Movement */
    void Accelerate(float Value);
    void Brake(float Value);
    void Steer(float Value);
    void ApplyFriction(float DeltaTime);

    /** Camera Controls */
    void LookUp(float Value);
    void TurnCamera(float Value);

    /** Speed & Physics */
    UPROPERTY(EditAnywhere, Category = "Vehicle Settings")
    float AccelerationRate;

    UPROPERTY(EditAnywhere, Category = "Vehicle Settings")
    float DecelerationRate;

    UPROPERTY(EditAnywhere, Category = "Vehicle Settings")
    float MaxSpeed;

    UPROPERTY(EditAnywhere, Category = "Vehicle Settings")
    float TurnSpeed;

    UPROPERTY(VisibleAnywhere, Category = "Vehicle Settings")
    float CurrentSpeed;

    UPROPERTY(EditAnywhere, Category = "Vehicle Settings")
    float FrictionCoefficient;

    /** Camera Components */
    UPROPERTY(VisibleAnywhere, Category = "Camera")
    USpringArmComponent* SpringArm;

    UPROPERTY(VisibleAnywhere, Category = "Camera")
    UCameraComponent* Camera;

    /** Camera Settings */
    UPROPERTY(EditAnywhere, Category = "Camera")
    float CameraSensitivity;

    UPROPERTY(EditAnywhere, Category = "Camera")
    float MaxLookUpAngle;

    UPROPERTY(EditAnywhere, Category = "Camera")
    float MaxLookDownAngle;

    float CameraPitch;

    /** UI Updates */
    void UpdateSpeedUI();
};
