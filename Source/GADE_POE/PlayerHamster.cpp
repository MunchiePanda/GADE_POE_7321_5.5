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
    PrimaryActorTick.bCanEverTick = true;

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
}

void APlayerHamster::BeginPlay()
{
    Super::BeginPlay();

    if (!Spline)
    {
        Spline = FindComponentByClass<USplineComponent>();
    }

    if (PauseMenuWidgetClass)
    {
        PauseMenuWidget = CreateWidget<UUserWidget>(GetWorld(), PauseMenuWidgetClass);
        if (PauseMenuWidget)
        {
            PauseMenuWidget->AddToViewport();
            PauseMenuWidget->SetVisibility(ESlateVisibility::Hidden);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create PauseMenuWidget!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PauseMenuClass is not set in the editor!"));
    }

    if (HUDClass)
    {
        HUDWidget = CreateWidget<UBeginnerRaceHUD>(GetWorld()->GetFirstPlayerController(), HUDClass);
        if (HUDWidget)
        {
            HUDWidget->AddToViewport();
        }
    }

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
}

void APlayerHamster::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (GameState && CurrentLap >= GameState->TotalLaps)
    {
        GetCharacterMovement()->DisableMovement();
        return;
    }

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
    if (Value != 0.0f)
    {
        AddControllerYawInput(Value * TurnSpeed * GetWorld()->GetDeltaSeconds());
    }
}

void APlayerHamster::Brake(float Value)
{
    if (Value > 0.0f)
    {
        CurrentSpeed = FMath::Clamp(CurrentSpeed - (BrakeForce * GetWorld()->GetDeltaSeconds()), 0.0f, MaxSpeed);
    }
}

void APlayerHamster::Turn(float Value)
{
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
    if (!PauseMenuWidget) return;

    bool bGamePaused = UGameplayStatics::IsGamePaused(GetWorld());
    if (bGamePaused)
    {
        PauseMenuWidget->SetVisibility(ESlateVisibility::Hidden);
        UGameplayStatics::SetGamePaused(GetWorld(), false);
        bIsPaused = false;
    }
    else
    {
        PauseMenuWidget->SetVisibility(ESlateVisibility::Visible);
        UGameplayStatics::SetGamePaused(GetWorld(), true);
        bIsPaused = true;
    }
}

float APlayerHamster::GetSpeed() const
{
    return CurrentSpeed;
}

void APlayerHamster::RegisterWithGameState()
{
    if (GameState)
    {
        GameState->RegisterRacer(this);
    }
}

void APlayerHamster::OnWaypointReached(AActor* Waypoint)
{
    CurrentWaypointIndex++;
    UE_LOG(LogTemp, Log, TEXT("PlayerHamster: Reached waypoint index %d"), CurrentWaypointIndex);

    if (GameState && CurrentWaypointIndex >= GameState->TotalWaypoints)
    {
        CurrentLap++;
        CurrentWaypointIndex = 0;
        UE_LOG(LogTemp, Log, TEXT("PlayerHamster: Completed lap %d"), CurrentLap);
    }

    if (GameState)
    {
        GameState->UpdateRacerProgress(this, CurrentLap, CurrentWaypointIndex);
    }
}