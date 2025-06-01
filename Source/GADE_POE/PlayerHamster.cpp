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
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    // Set up the visual mesh
    HamsterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HamsterMesh"));
    HamsterMesh->SetupAttachment(GetCapsuleComponent());
    HamsterMesh->SetCollisionProfileName(TEXT("NoCollision"));

    // Set up the physics body
    PhysicsBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PhysicsBody"));
    PhysicsBody->SetupAttachment(GetCapsuleComponent());
    PhysicsBody->SetSimulatePhysics(true);
    PhysicsBody->SetEnableGravity(true);
    PhysicsBody->SetConstraintMode(EDOFMode::Default);
    PhysicsBody->SetAngularDamping(5.0f); // Reduce unwanted rotations
    PhysicsBody->SetLinearDamping(0.5f);
    PhysicsBody->SetCollisionProfileName(TEXT("Pawn"));
    PhysicsBody->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    PhysicsBody->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    PhysicsBody->OnComponentBeginOverlap.AddDynamic(this, &APlayerHamster::OnPhysicsBodyOverlapBegin);

    // Configure the character movement component
    GetCharacterMovement()->MaxWalkSpeed = 600.f;
    GetCharacterMovement()->GravityScale = 0.0f; // Let physics handle gravity
    GetCharacterMovement()->bOrientRotationToMovement = false; // Allow physics to handle rotation
    GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
    GetCharacterMovement()->SetMovementMode(MOVE_Flying); // Use flying mode to let physics body interact with environment

    // Set up the spring arm component
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(GetCapsuleComponent());
    SpringArm->TargetArmLength = 300.0f;
    SpringArm->bEnableCameraLag = true;
    SpringArm->CameraLagSpeed = 5.0f;
    SpringArm->SetRelativeRotation(FRotator(-10.f, 0.f, 0.f));

    // Set up the camera
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm);

    // Set up the spline component
    CurrentLap = 0;
    CurrentWaypointIndex = 0;
    bIsPlayer = true;
    bUseGraphNavigation = false;
    CurrentWaypoint = nullptr;
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
        // Remove the PIE prefix if present (e.g., "UEDPIE_0_")
        CurrentLevelName.RemoveFromStart(TEXT("UEDPIE_0_"));
        UE_LOG(LogTemp, Log, TEXT("PlayerHamster: Current map name after prefix removal: %s"), *CurrentLevelName);

        // Use Contains for more robust matching
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
        FVector StartLocation = FirstWaypoint->GetActorLocation() + FVector(0.f, 0.f, 100.f); // Offset to avoid collision issues
        bool bTeleportSuccess = SetActorLocation(StartLocation, false, nullptr, ETeleportType::TeleportPhysics);
        if (bTeleportSuccess) // Check if the teleport was successful 
        {
            UE_LOG(LogTemp, Log, TEXT("PlayerHamster: Teleported to first waypoint at %s"), *StartLocation.ToString());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("PlayerHamster: Failed to teleport to first waypoint at %s"), *StartLocation.ToString());
        }
        CurrentWaypoint = FirstWaypoint; // Set initial waypoint for graph navigation
        UE_LOG(LogTemp, Log, TEXT("PlayerHamster: Current location after teleport attempt: %s"), *GetActorLocation().ToString());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerHamster: First waypoint is null"));
    }

    RegisterWithGameState(); // Register with the game state

    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController(); // Get the player controller
    if (PlayerController) // Check if the player controller is valid
    {
        PlayerController->SetInputMode(FInputModeGameOnly()); // Set input mode to game only
        PlayerController->bShowMouseCursor = false;
    }

    ASFXManager* SFXManager = ASFXManager::GetInstance(GetWorld()); // Get the SFXManager instance
    if (SFXManager)
    {
        SFXManager->PlayBackgroundMusic("bgm");
        UE_LOG(LogTemp, Log, TEXT("PlayerHamster: Started background music")); // Log message for debugging purposes
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerHamster: SFXManager is null!"));
    }
}

void APlayerHamster::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    ASFXManager* SFXManager = ASFXManager::GetInstance(GetWorld()); // Get the SFXManager instance

    if (GameState && CurrentLap >= GameState->TotalLaps) // Check if the player has finished the race 
    {
		GetCharacterMovement()->DisableMovement(); // Disable movement
        GameState->bRaceFinished = true; // Set the race finished flag
    }

    if (GameState && GameState->bRaceFinished && !bEndUIShown && EndUIWidget) // Check if the race has finished and the end UI has not been shown
    {
        EndUIWidget->AddToViewport(); // Add the end UI widget to the viewport
        EndUIWidget->SetVisibility(ESlateVisibility::Visible); // Set the visibility of the end UI widget
        APlayerController* PlayerController = GetWorld()->GetFirstPlayerController(); // Get the player controller
        if (PlayerController) // Check if the player controller is valid
        {
			PlayerController->SetInputMode(FInputModeUIOnly()); // Set input mode to UI only
            PlayerController->bShowMouseCursor = true; // Show the mouse cursor
        }
		if (SFXManager) // Check if the SFXManager is valid
        {
            SFXManager->PlaySound("end");
            SFXManager->StopBackgroundMusic();
        }
        bEndUIShown = true;
		UE_LOG(LogTemp, Log, TEXT("PlayerHamster: End UI shown")); // Log message for debugging purposes
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

    // Check if the target waypoint is valid and trigger OnWaypointReached if close enough
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

    // Apply physics force to the physics body
    if (MoveDirection.SizeSquared() > 0.0f)
    {
        FVector Force = MoveDirection * CurrentSpeed;
        PhysicsBody->AddForce(Force);
    }
}

// Called to bind functionality to input 
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

void APlayerHamster::MoveForward(float Value) // Function to move the player forward
{
    if (Value > 0.0f)
    {
        CurrentSpeed = FMath::Clamp(CurrentSpeed + (AccelerationRate * GetWorld()->GetDeltaSeconds()), 0.0f, MaxSpeed);
        MoveDirection = GetActorForwardVector();
    }
    else if (Value < 0.0f)
    {
        CurrentSpeed = FMath::Clamp(CurrentSpeed - (DecelerationRate * GetWorld()->GetDeltaSeconds()), 0.0f, MaxSpeed);
        MoveDirection = -GetActorForwardVector();
    }
    else
    {
        CurrentSpeed = FMath::Clamp(CurrentSpeed - (DecelerationRate * GetWorld()->GetDeltaSeconds()), 0.0f, MaxSpeed);
        MoveDirection = FVector::ZeroVector;
    }

    AddMovementInput(MoveDirection, CurrentSpeed * GetWorld()->GetDeltaSeconds());
}

void APlayerHamster::MoveRight(float Value) // Function to move the player right
{
    if (Value != 0.0f) 
    {
        AddControllerYawInput(Value * TurnSpeed * GetWorld()->GetDeltaSeconds());
        PhysicsBody->SetPhysicsAngularVelocityInDegrees(FVector(0, Value * TurnSpeed, 0)); // Apply angular velocity for turning
    }
}

void APlayerHamster::Brake(float Value) // Function to brake the player
{
    if (Value > 0.0f)
    {
        CurrentSpeed = FMath::Clamp(CurrentSpeed - (BrakeForce * GetWorld()->GetDeltaSeconds()), 0.0f, MaxSpeed);
        MoveDirection = FVector::ZeroVector;
    }
}

void APlayerHamster::Turn(float Value) // Function to turn the player
{
    if (SpringArm)
    {
        FRotator NewRotation = SpringArm->GetRelativeRotation();
        NewRotation.Yaw += Value;
        SpringArm->SetRelativeRotation(NewRotation);
    }
}

void APlayerHamster::LookUp(float Value) // Function to look up or down
{
    if (SpringArm)
    {
        FRotator NewRotation = SpringArm->GetRelativeRotation();
        NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch + Value, -80.0f, 10.0f);
        SpringArm->SetRelativeRotation(NewRotation);
    }
}

void APlayerHamster::TogglePauseMenu() // Function to toggle the pause menu
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

float APlayerHamster::GetSpeed() const // Function to get the current speed
{
    return CurrentSpeed;
}

void APlayerHamster::RegisterWithGameState() // Function to register the player hamster with the game state
{
    if (GameState)
    {
        GameState->RegisterRacer(this);
    }
}

void APlayerHamster::OnWaypointReached(AActor* Waypoint) // Function to handle when a waypoint is reached (advanced map)
{
    ASFXManager* SFXManager = ASFXManager::GetInstance(GetWorld());
    if (SFXManager)
    {
        SFXManager->PlaySound("waypoint");
    }

    if (bUseGraphNavigation && RaceManager)
    {
        // Graph-based navigation for advanced map
        AGraph* Graph = RaceManager->GetGraph();
        if (Graph)
        {
            TArray<AActor*> Neighbors = Graph->GetNeighbors(Waypoint);
            if (Neighbors.Num() > 0)
            {
                // For simplicity, take the first neighbor (handles branches like 3->4 or 3->5)
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
        // Original linear navigation for other levels
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

// Function to handle when the player hamster collides with another actor
void APlayerHamster::OnPhysicsBodyOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    ASFXManager* SFXManager = ASFXManager::GetInstance(GetWorld());
    if (OtherActor != this && OtherActor->GetName().Contains("Racer") && SFXManager)
    {
        SFXManager->PlaySound("crash");
        UE_LOG(LogTemp, Warning, TEXT("PlayerHamster: Collided with AI racer: %s"), *OtherActor->GetName());
    }
}