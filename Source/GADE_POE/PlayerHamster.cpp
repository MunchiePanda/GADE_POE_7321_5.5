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
    PhysicsBody->SetAngularDamping(0.1f);
    PhysicsBody->SetLinearDamping(0.05f);
    PhysicsBody->SetCollisionProfileName(TEXT("Pawn"));
    PhysicsBody->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    PhysicsBody->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    PhysicsBody->OnComponentBeginOverlap.AddDynamic(this, &APlayerHamster::OnPhysicsBodyOverlapBegin);
    PhysicsBody->SetConstraintMode(EDOFMode::Default);

    // Configure the character movement component for walking
    GetCharacterMovement()->MaxWalkSpeed = 600.f;
    GetCharacterMovement()->GravityScale = 2.0f;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
    GetCharacterMovement()->SetMovementMode(MOVE_Walking);

    // Set up the spring arm component for racing camera
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(GetCapsuleComponent());
    SpringArm->TargetArmLength = 300.0f;
    SpringArm->SocketOffset = FVector(0.0f, 0.0f, 100.0f);
    SpringArm->bEnableCameraLag = true;
    SpringArm->CameraLagSpeed = 5.0f;
    SpringArm->bEnableCameraRotationLag = true;
    SpringArm->CameraRotationLagSpeed = 10.0f;
    SpringArm->bInheritPitch = true;
    SpringArm->bInheritRoll = true;
    SpringArm->bInheritYaw = true;
    SpringArm->SetRelativeRotation(FRotator(-10.f, 0.f, 0.f));
    SpringArm->bDoCollisionTest = true;

    // Set up the camera
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
    Camera->bUsePawnControlRotation = false;

    // Set up the spline component
    CurrentLap = 0;
    CurrentWaypointIndex = 0;
    CurrentSpeed = 0.0f;
    bIsPlayer = true;
    bUseGraphNavigation = false;
    CurrentWaypoint = nullptr;

    // Ensure TurnSpeed is set to a reasonable value
    TurnSpeed = 100.0f;

    bIsPaused = false;
    WaypointManager = nullptr;
    RaceManager = nullptr;
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

    // Set up overlap events for waypoints
    GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &APlayerHamster::OnWaypointOverlap);

    // Find the race manager
    RaceManager = Cast<AAdvancedRaceManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AAdvancedRaceManager::StaticClass()));
    if (RaceManager)
    {
        bUseGraphNavigation = true;
        UE_LOG(LogTemp, Log, TEXT("PlayerHamster: Using graph navigation with AdvancedRaceManager"));
    }
    else
    {
        // Find waypoint manager as fallback
        WaypointManager = Cast<AWaypointManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AWaypointManager::StaticClass()));
        if (WaypointManager)
        {
            UE_LOG(LogTemp, Log, TEXT("PlayerHamster: Using waypoint manager navigation"));
        }
    }

    GameState = Cast<ABeginnerRaceGameState>(GetWorld()->GetGameState());
    if (!GameState)
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerHamster: GameState not found!"));
    }
    else
    {
        // Register the player with the GameState
        GameState->RegisterRacer(this);
        UE_LOG(LogTemp, Warning, TEXT("PlayerHamster: Registered with GameState"));
    }

    // Initialize the first waypoint for tracking (but don't teleport to it)
    AActor* FirstWaypoint = nullptr;
    if (bUseGraphNavigation && RaceManager)
    {
        FirstWaypoint = RaceManager->GetWaypoint(0);
    }
    else if (WaypointManager)
    {
        FirstWaypoint = WaypointManager->GetWaypoint(0);
    }

    if (FirstWaypoint)
    {
        CurrentWaypoint = FirstWaypoint;
        UE_LOG(LogTemp, Log, TEXT("PlayerHamster: First waypoint set to %s"), *FirstWaypoint->GetName());
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

    // Visualize waypoint choices if waiting for player input
    if (bWaitingForWaypointChoice && AvailableWaypoints.Num() > 0)
    {
        for (int32 i = 0; i < AvailableWaypoints.Num(); i++)
        {
            if (AvailableWaypoints[i])
            {
                FColor Color = (i == CurrentWaypointChoice) ? FColor::Green : FColor::Red;
                DrawDebugSphere(
                    GetWorld(),
                    AvailableWaypoints[i]->GetActorLocation(),
                    200.0f,  // Radius
                    12,      // Segments
                    Color,
                    false,   // Persistent
                    0.0f    // Duration (0 for single frame)
                );

                // Draw line from current position to each waypoint
                DrawDebugLine(
                    GetWorld(),
                    GetActorLocation(),
                    AvailableWaypoints[i]->GetActorLocation(),
                    Color,
                    false,
                    0.0f,
                    0,
                    5.0f  // Line thickness
                );
            }
        }
    }
    // Check distance to current waypoint and trigger OnWaypointReached if close enough
    else if (!bWaitingForWaypointChoice)  // Only check distance if not waiting for choice
    {
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

            // Only draw current waypoint debug sphere if not choosing
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
    
    // Add new input bindings for waypoint selection
    PlayerInputComponent->BindAction("SelectNextWaypoint", IE_Pressed, this, &APlayerHamster::SelectNextWaypoint);
    PlayerInputComponent->BindAction("ConfirmWaypoint", IE_Pressed, this, &APlayerHamster::ConfirmWaypoint);
}

void APlayerHamster::MoveForward(float Value)
{
    if (bIsPaused) return;

    if (Value > 0.0f)
    {
        CurrentSpeed = FMath::Clamp(CurrentSpeed + (AccelerationRate * GetWorld()->GetDeltaSeconds()), 0.0f, MaxSpeed);
        AddMovementInput(GetActorForwardVector(), CurrentSpeed * GetWorld()->GetDeltaSeconds());
    }
    else
    {
        CurrentSpeed = FMath::Clamp(CurrentSpeed - (DecelerationRate * GetWorld()->GetDeltaSeconds()), 0.0f, MaxSpeed);
        if (CurrentSpeed > 0.0f)
        {
            AddMovementInput(GetActorForwardVector(), CurrentSpeed * GetWorld()->GetDeltaSeconds());
        }
    }
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
    // Rotate the camera
    if (SpringArm)
    {
        FRotator NewRotation = SpringArm->GetRelativeRotation();
        NewRotation.Yaw += Value;
        SpringArm->SetRelativeRotation(NewRotation);
    }
}

void APlayerHamster::Turn(float Value)
{
    if (Value != 0.0f && !bIsPaused)
    {
        // Rotate both the camera and character
        AddControllerYawInput(Value);
        
        // Update character rotation to match controller rotation
        FRotator NewRotation = FRotator(0, Controller->GetControlRotation().Yaw, 0);
        SetActorRotation(NewRotation);
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
    if (!Waypoint)
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerHamster: OnWaypointReached called with null waypoint"));
        return;
    }

    ASFXManager* SFXManager = ASFXManager::GetInstance(GetWorld());
    if (SFXManager)
    {
        SFXManager->PlaySound("waypoint");
    }

    if (bUseGraphNavigation && RaceManager)
    {
        // Advanced map - Graph-based navigation
        for (int32 i = 0; i < RaceManager->GetTotalWaypoints(); ++i)
        {
            if (RaceManager->GetWaypoint(i) == Waypoint)
            {
                CurrentWaypointIndex = i;
                break;
            }
        }

        // Get next possible waypoints
        AGraph* Graph = RaceManager->GetGraph();
        if (Graph)
        {
            TArray<AActor*> Neighbors = Graph->GetNeighbors(Waypoint);
            if (Neighbors.Num() > 0)
            {
                CurrentWaypoint = Neighbors[0];
                UE_LOG(LogTemp, Log, TEXT("PlayerHamster: Moving to next waypoint %s"), *CurrentWaypoint->GetName());
            }
        }

        // Check if we've completed a lap (when we reach waypoint 11, which connects back to 0)
        if (CurrentWaypointIndex == 11)
        {
            CurrentLap++;
            CurrentWaypointIndex = 0;
            if (SFXManager)
            {
                SFXManager->PlaySound("lap");
            }
            UE_LOG(LogTemp, Log, TEXT("PlayerHamster: Completed lap %d"), CurrentLap);
        }
    }
    else
    {
        // Beginner map - Sequential waypoints
        CurrentWaypointIndex++;
        UE_LOG(LogTemp, Log, TEXT("PlayerHamster: Reached waypoint index %d"), CurrentWaypointIndex);

        // Check if a lap is completed
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
    }

    // Update GameState with progress
    if (GameState)
    {
        GameState->UpdateRacerProgress(this, CurrentLap, CurrentWaypointIndex);
        UE_LOG(LogTemp, Log, TEXT("PlayerHamster: Updated GameState - Lap: %d, WaypointIndex: %d"), CurrentLap, CurrentWaypointIndex);
    }
}

void APlayerHamster::SelectNextWaypoint()
{
    if (!bWaitingForWaypointChoice || AvailableWaypoints.Num() == 0)
        return;

    // Cycle to the next waypoint choice
    CurrentWaypointChoice = (CurrentWaypointChoice + 1) % AvailableWaypoints.Num();
    
    // Update debug visualization
    for (int32 i = 0; i < AvailableWaypoints.Num(); i++)
    {
        if (AvailableWaypoints[i])
        {
            FColor Color = (i == CurrentWaypointChoice) ? FColor::Green : FColor::Red;
            DrawDebugSphere(
                GetWorld(),
                AvailableWaypoints[i]->GetActorLocation(),
                200.0f,  // Radius
                12,      // Segments
                Color,
                false,   // Persistent
                0.1f    // Duration
            );
        }
    }
}

void APlayerHamster::ConfirmWaypoint()
{
    if (!bWaitingForWaypointChoice || AvailableWaypoints.Num() == 0)
        return;

    // Set the chosen waypoint as the current target
    CurrentWaypoint = AvailableWaypoints[CurrentWaypointChoice];
    bWaitingForWaypointChoice = false;
    
    // Find the index of the chosen waypoint in the RaceManager's waypoint list
    if (RaceManager)
    {
        for (int32 i = 0; i < RaceManager->GetTotalWaypoints(); ++i)
        {
            if (RaceManager->GetWaypoint(i) == CurrentWaypoint)
            {
                CurrentWaypointIndex = i;
                break;
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("PlayerHamster: Confirmed waypoint choice: %s (index: %d)"), 
        *CurrentWaypoint->GetName(), CurrentWaypointIndex);

    // Update game state with new progress
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

void APlayerHamster::OnWaypointOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (bIsPaused) return;

    AWaypoint* Waypoint = Cast<AWaypoint>(OtherActor);
    if (!Waypoint) return;

    // Log waypoint progression
    UE_LOG(LogTemp, Warning, TEXT("PLAYER - Current Waypoint: %s"), *Waypoint->GetName());
    
    // Call OnWaypointReached to handle waypoint progression and choices
    OnWaypointReached(Waypoint);

    // Debug visualization for the current waypoint
    DrawDebugSphere(
        GetWorld(),
        Waypoint->GetActorLocation(),
        200.0f,  // Radius
        12,      // Segments
        FColor::Yellow,
        false,   // Persistent
        2.0f    // Duration - longer duration to make it more visible
    );
    
    if (bUseGraphNavigation && RaceManager)
    {
        // Get next waypoint options from graph and visualize them
        TArray<AActor*> NextWaypoints = RaceManager->GetGraph()->GetNeighbors(Waypoint);
        
        FString NextOptionsStr;
        for (AActor* Next : NextWaypoints)
        {
            NextOptionsStr += FString::Printf(TEXT("%s, "), *Next->GetName());
            
            // Visualize available next waypoints
            DrawDebugSphere(
                GetWorld(),
                Next->GetActorLocation(),
                150.0f,  // Slightly smaller radius for next waypoints
                12,      // Segments
                FColor::Blue,
                false,   // Persistent
                2.0f    // Duration
            );

            // Draw lines to show connections
            DrawDebugLine(
                GetWorld(),
                Waypoint->GetActorLocation(),
                Next->GetActorLocation(),
                FColor::Green,
                false,
                2.0f,
                0,
                5.0f  // Line thickness
            );
        }
        
        UE_LOG(LogTemp, Warning, TEXT("PLAYER - Next Possible Waypoints: %s"), *NextOptionsStr);
    }
    else if (WaypointManager && WaypointManager->WaypointList)
    {
        AActor* NextWaypoint = WaypointManager->WaypointList->GetNext(Waypoint);
        if (NextWaypoint)
        {
            UE_LOG(LogTemp, Warning, TEXT("PLAYER - Next Waypoint: %s"), *NextWaypoint->GetName());
        }
    }

    // Update current waypoint
    CurrentWaypoint = Waypoint;
}

void APlayerHamster::SetSpeed(float NewSpeed)
{
    CurrentSpeed = FMath::Clamp(NewSpeed, 0.0f, MaxSpeed);
}