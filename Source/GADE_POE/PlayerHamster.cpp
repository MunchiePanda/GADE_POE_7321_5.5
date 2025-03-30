#include "PlayerHamster.h"
#include "Components/StaticMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "Components/SplineComponent.h"
#include "Blueprint/UserWidget.h"
#include "RaceHUDWidget.h"
#include "GameFramework/HUD.h"

// Sets default values
APlayerHamster::APlayerHamster()
{
    PrimaryActorTick.bCanEverTick = true;

    //Ensure the Capsule Component is the Root Component
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

    //Attach the Hamster Mesh to the Capsule
    HamsterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HamsterMesh"));
    HamsterMesh->SetupAttachment(GetCapsuleComponent());

    //Configure Character Movement Properly
    GetCharacterMovement()->MaxWalkSpeed = 600.f;
    GetCharacterMovement()->GravityScale = 2.0f;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);

    //Spring Arm (for smooth camera movement)
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(GetCapsuleComponent());
    SpringArm->TargetArmLength = 300.0f;
    SpringArm->bEnableCameraLag = true;
    SpringArm->CameraLagSpeed = 5.0f;
    SpringArm->SetRelativeRotation(FRotator(-10.f, 0.f, 0.f));

    //Camera
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm);
}


// Called when the game starts or when spawned
void APlayerHamster::BeginPlay()
{
    Super::BeginPlay();
    if (!Spline)
    {
        // Example: Look for the first SplineComponent attached to this actor.
        Spline = FindComponentByClass<USplineComponent>();
    }

}

// Called every frame
void APlayerHamster::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (Spline)
    {
        FVector CurrentLocation = GetActorLocation();
        FVector ClosestSplineLocation = Spline->FindLocationClosestToWorldLocation(CurrentLocation, ESplineCoordinateSpace::World);
        float DistanceToSpline = FVector::Distance(CurrentLocation, ClosestSplineLocation);

        if (DistanceToSpline > MaxDistanceFromSpline)
        {
            // Snap the hamster to the closest spline location
            SetActorLocation(ClosestSplineLocation);
        }

        // You could also have the hamster follow the spline as it moves forward:
        float SplineLength = Spline->GetSplineLength();
        float NewDistanceAlongSpline = FMath::Clamp(CurrentLocation.X, 0.0f, SplineLength); // Assuming movement is along X-axis
        FVector NewLocation = Spline->GetLocationAtDistanceAlongSpline(NewDistanceAlongSpline, ESplineCoordinateSpace::World);

        SetActorLocation(NewLocation);
    }

}

// Called to bind functionality to input
void APlayerHamster::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);  //This is required

    PlayerInputComponent->BindAxis("MoveForward", this, &APlayerHamster::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &APlayerHamster::MoveRight);
    PlayerInputComponent->BindAxis("Brake", this, &APlayerHamster::Brake);

    PlayerInputComponent->BindAxis("Turn", this, &APlayerHamster::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &APlayerHamster::LookUp);

	PlayerInputComponent->BindAction("Pause", IE_Pressed, this, &APlayerHamster::TogglePauseMenu);
}

void APlayerHamster::MoveForward(float Value)
{
    if (Value > 0.0f)
    {
        // Accelerate smoothly
        CurrentSpeed = FMath::Clamp(CurrentSpeed + (AccelerationRate * GetWorld()->GetDeltaSeconds()), 0.0f, MaxSpeed);
    }
    else
    {
        // Natural Deceleration
        CurrentSpeed = FMath::Clamp(CurrentSpeed - (DecelerationRate * GetWorld()->GetDeltaSeconds()), 0.0f, MaxSpeed);
    }

    // Apply movement based on CurrentSpeed
    AddMovementInput(GetActorForwardVector(), CurrentSpeed * GetWorld()->GetDeltaSeconds());
}

void APlayerHamster::MoveRight(float Value)
{
    if (Value != 0.0f)
    {
        AddControllerYawInput(Value * TurnSpeed * GetWorld()->GetDeltaSeconds());
    }
}

void APlayerHamster::Brake(float Value)
{
    if (Value > 0.0f)
    {
        // Apply brake force
        CurrentSpeed = FMath::Clamp(CurrentSpeed - (BrakeForce * GetWorld()->GetDeltaSeconds()), 0.0f, MaxSpeed);
    }
}
// Camera Controls
void APlayerHamster::Turn(float Value)
{

    if (SpringArm)
    {
		FRotator NewRotation = SpringArm->GetRelativeRotation();
		NewRotation.Yaw += Value;
		SpringArm->SetRelativeRotation(NewRotation);
    }
}

// Camera Controls
void APlayerHamster::LookUp(float Value)
{

    if (SpringArm)
    {
        FRotator NewRotation = SpringArm->GetRelativeRotation();
        NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch + Value, -80.0f, 10.0f);
		SpringArm->SetRelativeRotation(NewRotation);
    }
}

void APlayerHamster::TogglePauseMenu() // Toggle Pause Menu
{
    // Get the pause menu class
    if (!PauseMenuClass) return;

    if (!PauseMenuWidget) // Check if the pause menu widget is already created
    {
        PauseMenuWidget = CreateWidget<UUserWidget>(GetWorld(), PauseMenuClass);
        if (PauseMenuWidget) // Check if the pause menu widget was successfully created
        {
            PauseMenuWidget->AddToViewport();
        }
    }

    bIsPaused = !bIsPaused; // Toggle the pause state
    APlayerController* PlayerController = Cast<APlayerController>(GetController());

    if (bIsPaused) // If the game is paused
    {
        UGameplayStatics::SetGamePaused(GetWorld(), true); // Pause the game
        PlayerController->SetInputMode(FInputModeUIOnly()); // Set the input mode
        PlayerController->bShowMouseCursor = true; // Show the mouse cursor
    }
    else
    {
        UGameplayStatics::SetGamePaused(GetWorld(), false); // Unpause the game
        PlayerController->SetInputMode(FInputModeGameOnly()); // Set the input mode
        PlayerController->bShowMouseCursor = false; // Hide the mouse cursor

        if (PauseMenuWidget) // Check if the pause menu widget was successfully created
        {
            PauseMenuWidget->RemoveFromParent(); // Remove the pause menu widget
        }
    }

}

float APlayerHamster::GetSpeed() const
{
	return CurrentSpeed;
}
