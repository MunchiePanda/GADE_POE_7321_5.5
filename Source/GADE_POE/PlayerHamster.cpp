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
#include "SFXManager.h"
#include "AdvancedRaceManager.h"
#include "Graph.h"

APlayerHamster::APlayerHamster()
{
    PrimaryActorTick.bCanEverTick = true;

    // Configure the capsule component
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    // Set up the visual mesh
    HamsterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HamsterMesh"));
    HamsterMesh->SetupAttachment(GetCapsuleComponent());
    HamsterMesh->SetCollisionProfileName(TEXT("NoCollision"));

    // Set up the physics body (for collision detection only)
    PhysicsBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PhysicsBody"));
    PhysicsBody->SetupAttachment(GetCapsuleComponent());
    PhysicsBody->SetSimulatePhysics(true);
    PhysicsBody->SetEnableGravity(true);
    PhysicsBody->SetConstraintMode(EDOFMode::Default);
    PhysicsBody->SetAngularDamping(0.1f);
    PhysicsBody->SetLinearDamping(0.05f);
    PhysicsBody->SetCollisionProfileName(TEXT("Pawn"));
    PhysicsBody->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    PhysicsBody->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    PhysicsBody->OnComponentBeginOverlap.AddDynamic(this, &APlayerHamster::OnPhysicsBodyOverlapBegin);
    // Lock rotation to prevent physics from interfering with character rotation
    PhysicsBody->SetConstraintMode(EDOFMode::SixDOF);
    PhysicsBody->SetAllPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
    PhysicsBody->SetConstraintMode(EDOFMode::XZPlane); // Allow movement in XZ, lock rotation

    // Configure the character movement component for walking
    GetCharacterMovement()->MaxWalkSpeed = 600.f;
    GetCharacterMovement()->GravityScale = 1.0f;
    GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...
    GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f); // ...at this rotation rate
    GetCharacterMovement()->SetMovementMode(MOVE_Walking);

    // Set up the spring arm component for racing camera
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(GetCapsuleComponent());
    SpringArm->TargetArmLength = 400.0f; // Set camera distance
    SpringArm->SocketOffset = FVector(0.0f, 0.0f, 100.0f); // Raise the camera
    SpringArm->bEnableCameraLag = true; // Enable smooth camera following
    SpringArm->CameraLagSpeed = 3.0f; // Adjust how quickly camera catches up
    SpringArm->bEnableCameraRotationLag = true; // Enable smooth rotation
    SpringArm->CameraRotationLagSpeed = 10.0f; // Adjust rotation smoothing
    SpringArm->bInheritPitch = true;
    SpringArm->bInheritRoll = true;
    SpringArm->bInheritYaw = true;
    SpringArm->SetRelativeRotation(FRotator(-15.0f, -90.0f, 0.0f)); // -90 degrees yaw to align with model's right side
    SpringArm->bDoCollisionTest = true; // Enable collision testing

    // Set up the camera
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
    Camera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

    // Set up the spline component
    CurrentLap = 0;
    CurrentWaypointIndex = 0;
    bIsPlayer = true;
    bUseGraphNavigation = false;
    CurrentWaypoint = nullptr;

    // Ensure TurnSpeed is set to a reasonable value
    TurnSpeed = 100.0f;
}

void APlayerHamster::BeginPlay()
{
    Super::BeginPlay();

    // Find the spline component
    if (!Spline)
    {
        Spline = FindComponentByClass<USplineComponent>();
    }

    // Create the UI widgets
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

    // Create the UI widgets for the End UI
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

    // Create the HUD widget for BeginnerMap or AdvancedMap
    if (HUDClass)
    {
        FString CurrentLevelName = GetWorld()->GetMapName();
        CurrentLevelName.RemoveFromStart(TEXT("UEDPIE_0_"));
        UE_LOG(LogTemp, Log, TEXT("PlayerHamster: Current map name after prefix removal: %s"), *CurrentLevelName);

        bool bIsBeginnerMap = CurrentLevelName.Contains(TEXT("BeginnerMap"), ESearchCase::IgnoreCase);
        bool bIsAdvancedMap = CurrentLevelName.Contains(TEXT("AdvancedMap"), ESearchCase::IgnoreCase);

        if (bIsBeginnerMap || bIsAdvancedMap)
        {
            HUDWidget = CreateWidget<UBeginnerRaceHUD>(GetWorld()->GetFirstPlayerController(), HUDClass);
            if (HUDWidget)
            {
                HUDWidget->AddToViewport();
                UE_LOG(LogTemp, Log, TEXT("PlayerHamster: HUD widget added to viewport for %s"), *CurrentLevelName);
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("PlayerHamster: Failed to create HUD widget for %s"), *CurrentLevelName);
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("PlayerHamster: Map name %s does not match BeginnerMap or AdvancedMap"), *CurrentLevelName);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerHamster: HUDClass is not set!"));
    }

    // Check for AdvancedRaceManager first (for advanced map)
    RaceManager = Cast<AAdvancedRaceManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AAdvancedRaceManager::StaticClass()));
    if (RaceManager)
    {
        bUseGraphNavigation = true;
        UE_LOG(LogTemp, Log, TEXT("PlayerHamster: Using graph-based navigation (AdvancedRaceManager found)"));
    }
    else
    {
        // Fall back to WaypointManager for other levels
        WaypointManager = Cast<AWaypointManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AWaypointManager::StaticClass()));
        if (!WaypointManager)
        {
            UE_LOG(LogTemp, Error, TEXT("PlayerHamster: Neither AdvancedRaceManager nor WaypointManager found!"));
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("PlayerHamster: Using linear navigation (WaypointManager found)"));
        }
    }

    GameState = Cast<ABeginnerRaceGameState>(GetWorld()->GetGameState());
    if (!GameState)
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerHamster: GameState not found!"));
    }

    // Teleport to first waypoint to ensure proximity
    AActor* FirstWaypoint = nullptr;
    if (bUseGraphNavigation && RaceManager)
    {
        FirstWaypoint = RaceManager->GetWaypoint(0);
    }
    else if (WaypointManager)
    {
        FirstWaypoint = WaypointManager->GetWaypoint(0);
    }

    if (FirstWaypoint) // Check if the first waypoint is valid
    {
        // Teleport to the first waypoint with a small offset 
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
        CurrentWaypoint = FirstWaypoint;
        UE_LOG(LogTemp, Log, TEXT("PlayerHamster: Current location after teleport attempt: %s"), *GetActorLocation().ToString());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerHamster: First waypoint is null"));
    }

    RegisterWithGameState();

    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (PlayerController)
    {
        PlayerController->SetInputMode(FInputModeGameOnly());
        PlayerController->bShowMouseCursor = false;
    }

    ASFXManager* SFXManager = ASFXManager::GetInstance(GetWorld());
    if (SFXManager)
    {
        SFXManager->PlayBackgroundMusic("bgm");
        UE_LOG(LogTemp, Log, TEXT("PlayerHamster: Started background music"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerHamster: SFXManager is null!"));
    }

    // Set initial rotation to match the model's forward direction
    SetActorRotation(FRotator(0.0f, 90.0f, 0.0f));
}

void APlayerHamster::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    ASFXManager* SFXManager = ASFXManager::GetInstance(GetWorld());

    // Only the player's lap count determines race completion
    if (GameState && bIsPlayer && CurrentLap >= GameState->TotalLaps)
    {
        GetCharacterMovement()->DisableMovement();
        GameState->bRaceFinished = true;
    }

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
        if (SFXManager)
        {
            SFXManager->PlaySound("end");
            SFXManager->StopBackgroundMusic();
        }
        bEndUIShown = true;
        UE_LOG(LogTemp, Log, TEXT("PlayerHamster: End UI shown"));
    }

    // Check distance to current waypoint and trigger OnWaypointReached if close enough
    AActor* TargetWaypoint = nullptr;
    if (bUseGraphNavigation && RaceManager && CurrentWaypoint)
    {
        TargetWaypoint = CurrentWaypoint;
    }
    else if (WaypointManager)
    {
        TargetWaypoint = WaypointManager->GetWaypoint(CurrentWaypointIndex);
    }

    if (TargetWaypoint)
    {
        FVector PlayerLocation = GetActorLocation();
        FVector WaypointLocation = TargetWaypoint->GetActorLocation();
        float Distance = FVector::Dist(PlayerLocation, WaypointLocation);
        UE_LOG(LogTemp, Log, TEXT("PlayerHamster: Distance to waypoint %s: %f"), *TargetWaypoint->GetName(), Distance);

        USphereComponent* WaypointCollision = Cast<USphereComponent>(TargetWaypoint->GetComponentByClass(USphereComponent::StaticClass()));
        if (WaypointCollision)
        {
            TArray<AActor*> OverlappingActors;
            WaypointCollision->GetOverlappingActors(OverlappingActors, APlayerHamster::StaticClass());
            if (OverlappingActors.Contains(this))
            {
                OnWaypointReached(TargetWaypoint);
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("PlayerHamster: Waypoint %s lacks USphereComponent, using distance check"), *TargetWaypoint->GetName());
            if (Distance < 5000.0f)
            {
                OnWaypointReached(TargetWaypoint);
            }
        }

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
        UE_LOG(LogTemp, Warning, TEXT("PlayerHamster: TargetWaypoint is null (Index: %d)"), CurrentWaypointIndex);
    }

    if (CurrentSpeed > 0.0f && SFXManager)
    {
        SFXManager->PlaySound("engine");
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
    if (Value != 0.0f)
    {
        // Use the adjusted forward vector for movement
        AddMovementInput(GetActorForwardVector(), Value);
        CurrentSpeed = FMath::Clamp(GetCharacterMovement()->Velocity.Size(), 0.0f, MaxSpeed);
    }
    else
    {
        CurrentSpeed = FMath::Clamp(CurrentSpeed - (DecelerationRate * GetWorld()->GetDeltaSeconds()), 0.0f, MaxSpeed);
        if (CurrentSpeed <= 0.01f)
        {
            CurrentSpeed = 0.0f;
        }
    }
}

void APlayerHamster::MoveRight(float Value)
{
    if (Value != 0.0f)
    {
        // Calculate the rotation based on input, accounting for the model's orientation
        FRotator NewRotation = GetActorRotation();
        NewRotation.Yaw += Value * TurnSpeed * GetWorld()->GetDeltaSeconds();
        
        // Set the new rotation
        SetActorRotation(NewRotation);
        
        // The camera will automatically follow due to spring arm inheritance settings
        CurrentSpeed = FMath::Clamp(GetCharacterMovement()->Velocity.Size(), 0.0f, MaxSpeed);
    }
}

void APlayerHamster::Brake(float Value)
{
    if (Value > 0.0f)
    {
        CurrentSpeed = FMath::Clamp(CurrentSpeed - (BrakeForce * GetWorld()->GetDeltaSeconds()), 0.0f, MaxSpeed);
        if (CurrentSpeed <= 0.01f)
        {
            CurrentSpeed = 0.0f;
            FVector CurrentVelocity = GetCharacterMovement()->Velocity;
            if (CurrentVelocity.SizeSquared() > 0.0f)
            {
                AddMovementInput(CurrentVelocity.GetSafeNormal(), -Value);
            }
        }
    }
}

void APlayerHamster::Turn(float Value)
{
    // We don't need this anymore as the camera follows the character's rotation
}

void APlayerHamster::LookUp(float Value)
{
    if (SpringArm && Value != 0.0f)
    {
        // Allow limited vertical camera adjustment
        FRotator NewRotation = SpringArm->GetRelativeRotation();
        NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch + Value, -30.0f, 0.0f);
        SpringArm->SetRelativeRotation(NewRotation);
    }
}

void APlayerHamster::TogglePauseMenu()
{
    if (!PauseMenuWidget) return;
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController) return;

    ASFXManager* SFXManager = ASFXManager::GetInstance(GetWorld());

    if (bIsPaused)
    {
        PauseMenuWidget->SetVisibility(ESlateVisibility::Hidden);
        PauseMenuWidget->RemoveFromParent();
        UGameplayStatics::SetGamePaused(GetWorld(), false);
        PlayerController->SetInputMode(FInputModeGameOnly());
        PlayerController->bShowMouseCursor = false;
        bIsPaused = false;
        if (SFXManager)
        {
            SFXManager->PlaySound("unpause");
            SFXManager->PlayBackgroundMusic("bgm");
        }
    }
    else
    {
        PauseMenuWidget->AddToViewport();
        PauseMenuWidget->SetVisibility(ESlateVisibility::Visible);
        UGameplayStatics::SetGamePaused(GetWorld(), true);
        PlayerController->SetInputMode(FInputModeUIOnly());
        PlayerController->bShowMouseCursor = true;
        bIsPaused = true;
        if (SFXManager)
        {
            SFXManager->PlaySound("pause");
            SFXManager->StopBackgroundMusic();
        }
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
    ASFXManager* SFXManager = ASFXManager::GetInstance(GetWorld());
    if (SFXManager)
    {
        SFXManager->PlaySound("waypoint");
    }

    if (bUseGraphNavigation && RaceManager)
    {
        AGraph* Graph = RaceManager->GetGraph();
        if (Graph)
        {
            TArray<AActor*> Neighbors = Graph->GetNeighbors(Waypoint);
            if (Neighbors.Num() > 0)
            {
                CurrentWaypoint = Neighbors[0];
                CurrentWaypointIndex++;
                UE_LOG(LogTemp, Log, TEXT("PlayerHamster: Reached waypoint %s, moving to %s (index %d)"),
                    *Waypoint->GetName(), *CurrentWaypoint->GetName(), CurrentWaypointIndex);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("PlayerHamster: Waypoint %s has no neighbors"), *Waypoint->GetName());
                CurrentWaypoint = RaceManager->GetWaypoint(0);
                CurrentWaypointIndex = 0;
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("PlayerHamster: Graph is null in RaceManager"));
        }
    }
    else
    {
        CurrentWaypointIndex++;
        UE_LOG(LogTemp, Log, TEXT("PlayerHamster: Reached waypoint index %d"), CurrentWaypointIndex);
    }

    if (GameState && CurrentWaypointIndex >= GameState->TotalWaypoints)
    {
        CurrentLap++;
        CurrentWaypointIndex = 0;
        if (SFXManager)
        {
            SFXManager->PlaySound("lap");
        }
        UE_LOG(LogTemp, Log, TEXT("PlayerHamster: Completed lap %d"), CurrentLap);
    }

    if (GameState)
    {
        GameState->UpdateRacerProgress(this, CurrentLap, CurrentWaypointIndex);
    }
}

void APlayerHamster::OnPhysicsBodyOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    ASFXManager* SFXManager = ASFXManager::GetInstance(GetWorld());
    if (OtherActor != this && OtherActor->GetName().Contains("Racer") && SFXManager)
    {
        SFXManager->PlaySound("crash");
        UE_LOG(LogTemp, Warning, TEXT("PlayerHamster: Collided with AI racer: %s"), *OtherActor->GetName());
    }
}