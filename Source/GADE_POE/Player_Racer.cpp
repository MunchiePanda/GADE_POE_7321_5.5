#include "Player_Racer.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "ChaosVehicleMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/InputComponent.h"
#include "FrontWheels.h"
#include "RearWheels.h"

// Constructor
APlayer_Racer::APlayer_Racer()
{
    PrimaryActorTick.bCanEverTick = true;
 // Create and configure the vehicle movement component
    VehicleMovement = CreateDefaultSubobject<UChaosWheeledVehicleMovementComponent>(TEXT("VehicleMovement"));
    VehicleMovement->SetIsReplicated(true);
    //VehicleMovement->UpdatedComponent = GetCapsuleComponent();

    if (VehicleMovement)
    {
		VehicleMovement->WheelSetups.Empty();

        FChaosWheelSetup FrontLeft;
		FrontLeft.BoneName = FName("FrontLeft");
        FrontLeft.WheelClass = UFrontWheels::StaticClass();
        VehicleMovement->WheelSetups.Add(FrontLeft);

		FChaosWheelSetup FrontRight;
		FrontRight.BoneName = FName("FrontRight");
		FrontRight.WheelClass = UFrontWheels::StaticClass();
		VehicleMovement->WheelSetups.Add(FrontRight);

		FChaosWheelSetup RearLeft;
		RearLeft.BoneName = FName("RearLeft");
		RearLeft.WheelClass = URearWheels::StaticClass();
		VehicleMovement->WheelSetups.Add(RearLeft);

		FChaosWheelSetup RearRight;
		RearRight.BoneName = FName("RearRight");
		RearRight.WheelClass = URearWheels::StaticClass();
		VehicleMovement->WheelSetups.Add(RearRight);
    }
   

    // Ensure the torque curve exists and modify it directly
    FRuntimeFloatCurve& TorqueCurve = VehicleMovement->EngineSetup.TorqueCurve;

    // Clear any existing keys (optional)
    TorqueCurve.GetRichCurve()->Reset();

    // Add points to the torque curve
    TorqueCurve.GetRichCurve()->AddKey(0.0f, 400.0f);  // At 0 RPM, 400 torque
    TorqueCurve.GetRichCurve()->AddKey(1000.0f, 500.0f); // At 1000 RPM, 500 torque
    TorqueCurve.GetRichCurve()->AddKey(5000.0f, 300.0f); // At 5000 RPM, 300 torque
    TorqueCurve.GetRichCurve()->AddKey(7000.0f, 150.0f); // At 7000 RPM, 150 torque

    // Set max torque
    VehicleMovement->EngineSetup.MaxTorque = 500.0f;
    

    // Create Spring Arm for Camera
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->TargetArmLength = 500.0f;
    SpringArm->bEnableCameraLag = true;
    SpringArm->CameraLagSpeed = 6.0f;
    SpringArm->bDoCollisionTest = true;

    // Create Camera
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm);

    // Set Default Vehicle Stats
    MaxThrottle = 1.0f;
    MaxSteerAngle = 0.7f; // Adjusted for better control at higher speeds
}

void APlayer_Racer::BeginPlay()
{
    Super::BeginPlay();
    APlayerController* PlayerController = Cast<APlayerController>(GetController());
    if (PlayerController)
    {
        PlayerController->Possess(this);
    }
}

// Called every frame
void APlayer_Racer::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}

// Bind Inputs
void APlayer_Racer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis("Throttle", this, &APlayer_Racer::ThrottleInput);
    PlayerInputComponent->BindAxis("Steer", this, &APlayer_Racer::SteerInput);
    PlayerInputComponent->BindAxis("LookUp", this, &APlayer_Racer::LookUp);
    PlayerInputComponent->BindAxis("Turn", this, &APlayer_Racer::Turn);

    PlayerInputComponent->BindAction("Handbrake", IE_Pressed, this, &APlayer_Racer::HandbrakePressed);
    PlayerInputComponent->BindAction("Handbrake", IE_Released, this, &APlayer_Racer::HandbrakeReleased);
}

// Acceleration & Braking
void APlayer_Racer::ThrottleInput(float Val)
{
    if (VehicleMovement)
    {
        VehicleMovement->SetThrottleInput(Val * MaxThrottle);
    }
  

}

// Steering (Speed-Sensitive)
void APlayer_Racer::SteerInput(float Val)
{
    if (VehicleMovement)
    {
        float SteeringFactor = FMath::Clamp(1.0f - (VehicleMovement->GetForwardSpeed() / 25000.0f), 0.4f, 1.0f);
        VehicleMovement->SetSteeringInput(Val * MaxSteerAngle * SteeringFactor);
    }
}

// Handbrake (Drifting)
void APlayer_Racer::HandbrakePressed()
{
    if (VehicleMovement)
    {
        VehicleMovement->SetHandbrakeInput(true);
    }
}

void APlayer_Racer::HandbrakeReleased()
{
    if (VehicleMovement)
    {
        VehicleMovement->SetHandbrakeInput(false);
    }
}

// Camera Controls
void APlayer_Racer::LookUp(float Val)
{
    AddControllerPitchInput(Val);
}

void APlayer_Racer::Turn(float Val)
{
    AddControllerYawInput(Val);
}
