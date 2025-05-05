#include "PlayerHamster.h"
#include "Components/StaticMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "Components/SplineComponent.h"
#include "Blueprint/UserWidget.h"
#include "WaypointManager.h"
#include "BiginnerRaceGameState.h"
#include "BeginnerRaceHUD.h"
#include "DrawDebugHelpers.h"
#include "Components/SphereComponent.h"

APlayerHamster::APlayerHamster()
{
    // Set this character to call Tick() every frame
    PrimaryActorTick.bCanEverTick = true;

    /*Set all the default values*/
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

    HamsterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HamsterMesh"));
    HamsterMesh->SetupAttachment(GetCapsuleComponent());

    GetCharacterMovement()->MaxWalkSpeed = 600.f;
    GetCharacterMovement()->GravityScale = 2.0f;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);

    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(GetCapsuleComponent());
    SpringArm->TargetArmLength = 300.0f;
    SpringArm->bEnableCameraLag = true;
    SpringArm->CameraLagSpeed = 5.0f;
    SpringArm->SetRelativeRotation(FRotator(-10.f, 0.f, 0.f));

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm);

    CurrentLap = 0;
    CurrentWaypointIndex = 0;
    bIsPlayer = true;
}

void APlayerHamster::BeginPlay()
{
    Super::BeginPlay();

    if (!Spline)
    {
        Spline = FindComponentByClass<USplineComponent>();
    }

    // Create PauseMenuWidget but don't add to viewport yet
    if (PauseMenuWidgetClass)
    {
        PauseMenuWidget = CreateWidget<UUserWidget>(GetWorld(), PauseMenuWidgetClass);
        if (!PauseMenuWidget)
        {
            UE_LOG(LogTemp, Error, TEXT("PlayerHamster: Failed to create PauseMenuWidget!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerHamster: PauseMenuWidgetClass is not set in the editor!"));
    }

    // Create EndUIWidget but don't add to viewport yet
    if (EndUIWidgetClass)
    {
        EndUIWidget = CreateWidget<UUserWidget>(GetWorld(), EndUIWidgetClass);
        if (!EndUIWidget)
        {
            UE_LOG(LogTemp, Error, TEXT("PlayerHamster: Failed to create EndUIWidget!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerHamster: EndUIWidgetClass is not set in the editor!"));
    }

    // Create HUD and add to viewport
    if (HUDClass)
    {
        HUDWidget = CreateWidget<UBeginnerRaceHUD>(GetWorld()->GetFirstPlayerController(), HUDClass);
        if (HUDWidget)
        {
            HUDWidget->AddToViewport();
        }
    }

    // Get total waypoints from WaypointManager and LinkedList
    WaypointManager = Cast<AWaypointManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AWaypointManager::StaticClass()));
    if (!WaypointManager)
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerHamster: WaypointManager not found!"));
    }

    GameState = Cast<ABeginnerRaceGameState>(GetWorld()->GetGameState());
    if (!GameState)
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerHamster: GameState not found!"));
    }

    // Teleport to first waypoint to ensure proximity
    if (WaypointManager)
    {
        AActor* FirstWaypoint = WaypointManager->GetWaypoint(0);
        if (FirstWaypoint)
        {
            FVector StartLocation = FirstWaypoint->GetActorLocation() + FVector(0.f, 0.f, 100.f);
            bool bTeleportSuccess = SetActorLocation(StartLocation, false, nullptr, ETeleportType::TeleportPhysics);
            if (bTeleportSuccess)
            {
                UE_LOG(LogTemp, Log, TEXT("PlayerHamster: Teleported to first waypoint at %s"), *StartLocation.ToString());
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("PlayerHamster: Failed to teleport to first waypoint at %s"), *StartLocation.ToString());
            }
            // Log current player location for debugging
            UE_LOG(LogTemp, Log, TEXT("PlayerHamster: Current location after teleport attempt: %s"), *GetActorLocation().ToString());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("PlayerHamster: First waypoint is null"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerHamster: WaypointManager is null, cannot teleport"));
    }

    RegisterWithGameState();

    // Ensure game input mode
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (PlayerController)
    {
        PlayerController->SetInputMode(FInputModeGameOnly());
        PlayerController->bShowMouseCursor = false;
    }
}

void APlayerHamster::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Check if the player has finished the race and disable movement
    if (GameState && CurrentLap >= GameState->TotalLaps)
    {
        GetCharacterMovement()->DisableMovement();
    }

    // Show End UI when race finishes
    if (GameState && GameState->bRaceFinished && !bEndUIShown && EndUIWidget)
    {
        EndUIWidget->AddToViewport();
        EndUIWidget->SetVisibility(ESlateVisibility::Visible);
        APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
        if (PlayerController)
        {
            PlayerController->SetInputMode(FInputModeUIOnly());
            PlayerController->bShowMouseCursor = true;
        }
        bEndUIShown = true;
        UE_LOG(LogTemp, Log, TEXT("PlayerHamster: End UI shown"));
    }

    // Check distance to current waypoint and trigger OnWaypointReached if close enough 
    if (WaypointManager)
    {
        AActor* CurrentWaypoint = WaypointManager->GetWaypoint(CurrentWaypointIndex);
        if (CurrentWaypoint)
        {
            FVector PlayerLocation = GetActorLocation();
            FVector WaypointLocation = CurrentWaypoint->GetActorLocation();
            float Distance = FVector::Dist(PlayerLocation, WaypointLocation);
            UE_LOG(LogTemp, Log, TEXT("PlayerHamster: Distance to waypoint %s: %f"), *CurrentWaypoint->GetName(), Distance);

            // Check for sphere component overlap
            USphereComponent* WaypointCollision = Cast<USphereComponent>(CurrentWaypoint->GetComponentByClass(USphereComponent::StaticClass()));
            if (WaypointCollision)
            {
                TArray<AActor*> OverlappingActors;
                WaypointCollision->GetOverlappingActors(OverlappingActors, APlayerHamster::StaticClass());
                if (OverlappingActors.Contains(this))
                {
                    OnWaypointReached(CurrentWaypoint);
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("PlayerHamster: Waypoint %s lacks USphereComponent, using distance check"), *CurrentWaypoint->GetName());
                // Fallback distance-based check
                if (Distance < 5000.0f)
                {
                    OnWaypointReached(CurrentWaypoint);
                }
            }

            // Debug visualization
            if (WaypointCollision)
            {
                float SphereRadius = WaypointCollision->GetScaledSphereRadius();
                DrawDebugSphere(GetWorld(), WaypointLocation, SphereRadius, 12, FColor::Green, false, 0.1f);
            }
            else
            {
                DrawDebugSphere(GetWorld(), WaypointLocation, 5000.0f, 12, FColor::Red, false, 0.1f);
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("PlayerHamster: CurrentWaypoint at index %d is null"), CurrentWaypointIndex);
        }
    }
}

void APlayerHamster::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Input bindings for movement
    PlayerInputComponent->BindAxis("MoveForward", this, &APlayerHamster::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &APlayerHamster::MoveRight);
    PlayerInputComponent->BindAxis("Brake", this, &APlayerHamster::Brake);
    // Input bindings for camera control
    PlayerInputComponent->BindAxis("Turn", this, &APlayerHamster::Turn);
    PlayerInputComponent->BindAxis("LookUp", this, &APlayerHamster::LookUp);
    // Input binding for pause
    PlayerInputComponent->BindAction("Pause", IE_Pressed, this, &APlayerHamster::TogglePauseMenu);
}

// Player movement
void APlayerHamster::MoveForward(float Value)
{
    if (Value > 0.0f)
    {
        CurrentSpeed = FMath::Clamp(CurrentSpeed + (AccelerationRate * GetWorld()->GetDeltaSeconds()), 0.0f, MaxSpeed);
    }
    else
    {
        CurrentSpeed = FMath::Clamp(CurrentSpeed - (DecelerationRate * GetWorld()->GetDeltaSeconds()), 0.0f, MaxSpeed);
    }

    AddMovementInput(GetActorForwardVector(), CurrentSpeed * GetWorld()->GetDeltaSeconds());
}

void APlayerHamster::MoveRight(float Value)
{
    // Rotate the player
    if (Value != 0.0f)
    {
        AddControllerYawInput(Value * TurnSpeed * GetWorld()->GetDeltaSeconds());
    }
}

void APlayerHamster::Brake(float Value)
{
    // Apply braking
    if (Value > 0.0f)
    {
        CurrentSpeed = FMath::Clamp(CurrentSpeed - (BrakeForce * GetWorld()->GetDeltaSeconds()), 0.0f, MaxSpeed);
    }
}

void APlayerHamster::Turn(float Value)
{
    // Rotate the camera
    if (SpringArm)
    {
        FRotator NewRotation = SpringArm->GetRelativeRotation();
        NewRotation.Yaw += Value;
        SpringArm->SetRelativeRotation(NewRotation);
    }
}

void APlayerHamster::LookUp(float Value)
{
    if (SpringArm)
    {
        FRotator NewRotation = SpringArm->GetRelativeRotation();
        NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch + Value, -80.0f, 10.0f);
        SpringArm->SetRelativeRotation(NewRotation);
    }
}

void APlayerHamster::TogglePauseMenu()
{
    // Check if the PauseMenuWidget is valid before proceeding with the toggle
    if (!PauseMenuWidget) return;
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController) return;
    if (bIsPaused)
    {
        PauseMenuWidget->SetVisibility(ESlateVisibility::Hidden); // Hide the widget when unpausing 
        PauseMenuWidget->RemoveFromParent();
        UGameplayStatics::SetGamePaused(GetWorld(), false); // Unpause the game 
        PlayerController->SetInputMode(FInputModeGameOnly());
        PlayerController->bShowMouseCursor = false; // Hide the mouse cursor
        bIsPaused = false;
    }
    else
    {
        PauseMenuWidget->AddToViewport(); // Add the widget to the viewport
        PauseMenuWidget->SetVisibility(ESlateVisibility::Visible); // Show the widget
        UGameplayStatics::SetGamePaused(GetWorld(), true); // Pause the game
        PlayerController->SetInputMode(FInputModeUIOnly()); // Set the input mode
        PlayerController->bShowMouseCursor = true; // Show the mouse cursor
        bIsPaused = true;
    }
}

float APlayerHamster::GetSpeed() const // Get the current speed
{
    return CurrentSpeed;
}

void APlayerHamster::RegisterWithGameState() // Register the racer with the game state
{
    if (GameState)
    {
        GameState->RegisterRacer(this); // Call the RegisterRacer function in the game state
    }
}

void APlayerHamster::OnWaypointReached(AActor* Waypoint) // Triggered when a waypoint is reached 
{
    CurrentWaypointIndex++; // Increment the current waypoint index
    UE_LOG(LogTemp, Log, TEXT("PlayerHamster: Reached waypoint index %d"), CurrentWaypointIndex);

    // Check if a lap is completed 
    if (GameState && CurrentWaypointIndex >= GameState->TotalWaypoints)
    {
        CurrentLap++;
        CurrentWaypointIndex = 0;
        UE_LOG(LogTemp, Log, TEXT("PlayerHamster: Completed lap %d"), CurrentLap);
    }

    if (GameState) // Update the game state
    {
        GameState->UpdateRacerProgress(this, CurrentLap, CurrentWaypointIndex); // Call the UpdateRacerProgress function in the game state
    }
}