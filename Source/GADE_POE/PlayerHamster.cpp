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
        PauseMenuWidget = CreateWidget<UPauseMenu_WB>(GetWorld(), PauseMenuWidgetClass);
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

    if (Spline)
    {
        FVector CurrentLocation = GetActorLocation();
        FVector ClosestSplineLocation = Spline->FindLocationClosestToWorldLocation(CurrentLocation, ESplineCoordinateSpace::World);
        float DistanceToSpline = FVector::Distance(CurrentLocation, ClosestSplineLocation);

        if (DistanceToSpline > MaxDistanceFromSpline)
        {
            SetActorLocation(ClosestSplineLocation);
        }

        float SplineLength = Spline->GetSplineLength();
        float NewDistanceAlongSpline = FMath::Clamp(CurrentLocation.X, 0.0f, SplineLength);
        FVector NewLocation = Spline->GetLocationAtDistanceAlongSpline(NewDistanceAlongSpline, ESplineCoordinateSpace::World);
        SetActorLocation(NewLocation);
    }

    if (WaypointManager)
    {
        AActor* CurrentWaypoint = WaypointManager->GetWaypoint(CurrentWaypointIndex);
        if (CurrentWaypoint)
        {
            float Distance = FVector::Dist(GetActorLocation(), CurrentWaypoint->GetActorLocation());
            if (Distance < 200.0f)
            {
                OnWaypointReached(CurrentWaypoint);
            }
        }
    }

    /*
    if (HUDWidget && GameState)
    {
        UTextBlock* LapCounter = Cast<UTextBlock>(HUDWidget->GetWidgetFromName("LapCounter"));
        if (LapCounter)
        {
            LapCounter->SetText(FText::FromString(FString::Printf(TEXT("Lap %d/%d"), CurrentLap, GameState->TotalLaps)));
        }
    }
	*/
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