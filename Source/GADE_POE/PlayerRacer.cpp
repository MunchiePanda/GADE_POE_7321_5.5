#include "PlayerRacer.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"
//#include "../../../../UE_5.4/Engine/Plugins/Runtime/Metasound/Source/MetasoundEditor/Private/MetasoundEditorGraphConnectionDrawingPolicy.h"

// Sets default values
APlayerRacer::APlayerRacer()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create Spring Arm for Camera
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->TargetArmLength = 350.0f;
    SpringArm->bEnableCameraLag = true;
    SpringArm->CameraLagSpeed = 4.0f;
    SpringArm->bDoCollisionTest = true;
    SpringArm->bUsePawnControlRotation = false;

    // Create Camera
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm);

    // Set Default Vehicle Stats
    AccelerationRate = 10000.0f;
    DecelerationRate = 12000.0f;
    MaxSpeed = 25000.0f;
    TurnSpeed = 120.0f;
    FrictionCoefficient = 0.98f;
    CurrentSpeed = 0.0f;

    // Camera Settings
    CameraSensitivity = 1.5f;
    MaxLookUpAngle = 10.0f;
    MaxLookDownAngle = -25.0f;
    CameraPitch = 0.0f;

    // Configure Character Movement to Feel Like a Car
    GetCharacterMovement()->MaxAcceleration = 12000.0f;
    GetCharacterMovement()->BrakingDecelerationWalking = 9000.0f;
    GetCharacterMovement()->bOrientRotationToMovement = false;
}

// Called when the game starts
void APlayerRacer::BeginPlay()
{
    Super::BeginPlay();
}

// Called every frame
void APlayerRacer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Apply Friction for Smooth Speed Decay
    ApplyFriction(DeltaTime);

    // Limit Speed
    CurrentSpeed = FMath::Clamp(CurrentSpeed, 0.0f, MaxSpeed);

    // Move Forward Based on Speed
    AddMovementInput(GetActorForwardVector(), CurrentSpeed * DeltaTime);

    UpdateSpeedUI();
}

// Input Bindings
void APlayerRacer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis("MoveForward", this, &APlayerRacer::Accelerate);
    PlayerInputComponent->BindAxis("Brake", this, &APlayerRacer::Brake);
    PlayerInputComponent->BindAxis("MoveRight", this, &APlayerRacer::Steer);
}

// Acceleration (Smooth, Racing Template-Like)
void APlayerRacer::Accelerate(float Value)
{
    if (Value > 0.1f)
    {
        CurrentSpeed += AccelerationRate * Value * GetWorld()->GetDeltaSeconds();
        CurrentSpeed = FMath::Clamp(CurrentSpeed, 0.0f, MaxSpeed);
    }
}

// Braking (Gradual Deceleration)
void APlayerRacer::Brake(float Value)
{
    if (Value > 0.1f)
    {
        CurrentSpeed -= DecelerationRate * Value * GetWorld()->GetDeltaSeconds();
        CurrentSpeed = FMath::Clamp(CurrentSpeed, 0.0f, MaxSpeed);
    }
}

// Steering (Speed-Dependent)
void APlayerRacer::Steer(float Value)
{
    if (Value != 0.0f && CurrentSpeed > 100.0f) // Only turn when moving
    {
        float SteeringFactor = FMath::Clamp(1.0f - (CurrentSpeed / MaxSpeed), 0.5f, 1.0f);

        // Rotate the character instead of just yaw input
        FRotator NewRotation = GetActorRotation();
        NewRotation.Yaw += Value * TurnSpeed * SteeringFactor * GetWorld()->GetDeltaSeconds();
        SetActorRotation(NewRotation);
    }
}


// Apply Friction (Slows Down Over Time)
void APlayerRacer::ApplyFriction(float DeltaTime)
{
    if (CurrentSpeed > 0.0f)
    {
        CurrentSpeed *= FMath::Pow(FrictionCoefficient, DeltaTime * 60.0f);
    }
}

// Speed UI Update
void APlayerRacer::UpdateSpeedUI()
{
    float SpeedKmh = CurrentSpeed / 100.0f;
    UE_LOG(LogTemp, Warning, TEXT("Speed: %.1f km/h"), SpeedKmh);
}
