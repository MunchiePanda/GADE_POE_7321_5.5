/**
 * Implementation of the AIRacer class
 * This file contains the core logic for AI-controlled racing characters,
 * including movement, physics, and racing behavior.
 */

#include "AIRacer.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "RacerTypes.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "AIRacerContoller.h"
#include "Kismet/GameplayStatics.h"
#include "BiginnerRaceGameState.h"
#include "NavigationSystem.h"
#include "AI/Navigation/NavigationTypes.h"

AAIRacer::AAIRacer()
{
    PrimaryActorTick.bCanEverTick = true;

    // Set up collision capsule
    UCapsuleComponent* Capsule = GetCapsuleComponent();
    if (Capsule)
    {
        Capsule->SetCapsuleSize(40.0f, 96.0f);
        Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Capsule->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
        
        // Configure collision responses
        FCollisionResponseContainer ResponseContainer;
        ResponseContainer.SetAllChannels(ECR_Block);
        ResponseContainer.SetResponse(ECC_Pawn, ECR_Overlap);  // Allow racer overlap
        ResponseContainer.SetResponse(ECC_Camera, ECR_Ignore); // Ignore camera
        Capsule->SetCollisionResponseToChannels(ResponseContainer);
        
        Capsule->SetNotifyRigidBodyCollision(true);
        Capsule->SetGenerateOverlapEvents(true);
    }

    // Set up visual mesh
    RacerMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RacerMesh"));
    RacerMesh->SetupAttachment(RootComponent);
    RacerMesh->SetCollisionProfileName(TEXT("NoCollision")); 

    // Set up physics body
    PhysicsBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PhysicsBody"));
    PhysicsBody->SetupAttachment(RootComponent);
    PhysicsBody->SetSimulatePhysics(false);
    PhysicsBody->SetEnableGravity(true);
    PhysicsBody->SetCollisionProfileName(TEXT("BlockAllDynamic"));

    // Set up movement
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (Movement)
    {
        // Basic movement settings
        Movement->bOrientRotationToMovement = true;
        Movement->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
        Movement->bUseControllerDesiredRotation = true;
        
        // Ground movement
        Movement->NavAgentProps.bCanWalk = true;
        Movement->NavAgentProps.bCanFly = false;
        Movement->bCanWalkOffLedges = false;
        Movement->SetMovementMode(MOVE_Walking);
        Movement->GravityScale = 8.0f;
        
        // Speed settings
        Movement->MaxWalkSpeed = 4000.0f;      // Base speed
        Movement->MaxAcceleration = 4000.0f;   // Base acceleration
        Movement->BrakingDecelerationWalking = 8000.0f;  // Braking power
        Movement->GroundFriction = 8.0f;       // Ground friction
        
        // Navigation settings
        Movement->NavAgentProps.AgentRadius = 50.0f;
        Movement->NavAgentProps.AgentHeight = 96.0f;
        Movement->bUseRVOAvoidance = true;
        Movement->AvoidanceConsiderationRadius = 200.0f;
        Movement->AvoidanceWeight = 1.0f;
        
        // Physics tweaks
        Movement->Mass = 100.0f;
        Movement->bMaintainHorizontalGroundVelocity = true;
        Movement->bSnapToPlaneAtStart = true;
        Movement->SetWalkableFloorAngle(50.0f);
        Movement->bAlwaysCheckFloor = true;
        Movement->bUseFlatBaseForFloorChecks = true;
        Movement->MinAnalogWalkSpeed = 0.0f;
        Movement->bEnablePhysicsInteraction = true;

        // Allow for speed boosts
        Movement->MaxCustomMovementSpeed = 4800.0f;
        Movement->bRequestedMoveUseAcceleration = true;
    }

    // Set up AI control
    AIControllerClass = AAIRacerContoller::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    // Default to medium racer
    RacerType = ERacerType::Medium;
    MaxSpeed = 4000.0f;
    MaxAcceleration = 4000.0f;
}

void AAIRacer::BeginPlay()
{
    Super::BeginPlay();
    
    // Set up racer-specific settings
    SetupRacerAttributes();

    // Register with game state
    GameState = Cast<ABeginnerRaceGameState>(GetWorld()->GetGameState());
    if (GameState)
    {
        GameState->RegisterRacer(this);
        UE_LOG(LogTemp, Log, TEXT("AIRacer %s: Registered with GameState"), *GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AIRacer %s: Failed to find BeginnerRaceGameState."), *GetName());
    }

    // Log initial setup
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (Movement)
    {
        UE_LOG(LogTemp, Log, TEXT("AIRacer %s Configuration:"), *GetName());
        UE_LOG(LogTemp, Log, TEXT("  - Type: %s"), *UEnum::GetValueAsString(RacerType));
        UE_LOG(LogTemp, Log, TEXT("  - MaxSpeed: %.2f"), Movement->MaxWalkSpeed);
        UE_LOG(LogTemp, Log, TEXT("  - MaxAcceleration: %.2f"), Movement->MaxAcceleration);
        UE_LOG(LogTemp, Log, TEXT("  - RotationRate: %s"), *Movement->RotationRate.ToString());
        UE_LOG(LogTemp, Log, TEXT("  - BrakingDeceleration: %.2f"), Movement->BrakingDecelerationWalking);
        UE_LOG(LogTemp, Log, TEXT("  - AvoidanceRadius: %.2f"), Movement->AvoidanceConsiderationRadius);
        UE_LOG(LogTemp, Log, TEXT("  - AvoidanceWeight: %.2f"), Movement->AvoidanceWeight);
        UE_LOG(LogTemp, Log, TEXT("  - AgentRadius: %.2f"), Movement->NavAgentProps.AgentRadius);
    }
}

void AAIRacer::SetupRacerAttributes()
{
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (!Movement) return;

    // Base stats
    float BaseSpeed = 4000.0f;
    float BaseAccel = 4000.0f;
    float AvoidanceRadius = 200.0f;

    UE_LOG(LogTemp, Log, TEXT("AIRacer %s: Setting up attributes"), *GetName());

    // Configure based on racer type
    switch (RacerType)
    {
    case ERacerType::Fast:
        // Fast racers: +20% speed/accel, better turning, stronger braking
        MaxSpeed = BaseSpeed * 1.2f;
        MaxAcceleration = BaseAccel * 1.2f;
        if (Movement)
        {
            Movement->AvoidanceConsiderationRadius = AvoidanceRadius * 1.5f;
            Movement->RotationRate = FRotator(0.0f, 900.0f, 0.0f);
            Movement->BrakingDecelerationWalking = 12000.0f;
            UE_LOG(LogTemp, Log, TEXT("AIRacer %s: Configured as Fast type"), *GetName());
        }
        break;

    case ERacerType::Medium:
        // Medium racers: Balanced stats
        MaxSpeed = BaseSpeed;
        MaxAcceleration = BaseAccel;
        if (Movement)
        {
            Movement->AvoidanceConsiderationRadius = AvoidanceRadius * 1.2f;
            Movement->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
            Movement->BrakingDecelerationWalking = 8000.0f;
            UE_LOG(LogTemp, Log, TEXT("AIRacer %s: Configured as Medium type"), *GetName());
        }
        break;

    case ERacerType::Slow:
        // Slow racers: -20% speed/accel, slower turning, gentler braking
        MaxSpeed = BaseSpeed * 0.8f;
        MaxAcceleration = BaseAccel * 0.8f;
        if (Movement)
        {
            Movement->AvoidanceConsiderationRadius = AvoidanceRadius;
            Movement->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
            Movement->BrakingDecelerationWalking = 6000.0f;
            UE_LOG(LogTemp, Log, TEXT("AIRacer %s: Configured as Slow type"), *GetName());
        }
        break;
    }

    // Apply settings
    if (Movement)
    {
        Movement->MaxWalkSpeed = MaxSpeed;
        Movement->MaxAcceleration = MaxAcceleration;
        Movement->MaxCustomMovementSpeed = MaxSpeed * 1.2f;  // Allow boost
        Movement->AvoidanceWeight = 1.0f;
        Movement->bUseRVOAvoidance = true;
        Movement->bRequestedMoveUseAcceleration = true;

        // Log final setup
        UE_LOG(LogTemp, Log, TEXT("AIRacer %s: Final Movement Configuration:"), *GetName());
        UE_LOG(LogTemp, Log, TEXT("  - MaxWalkSpeed: %.2f"), Movement->MaxWalkSpeed);
        UE_LOG(LogTemp, Log, TEXT("  - MaxAcceleration: %.2f"), Movement->MaxAcceleration);
        UE_LOG(LogTemp, Log, TEXT("  - MaxCustomSpeed: %.2f"), Movement->MaxCustomMovementSpeed);
        UE_LOG(LogTemp, Log, TEXT("  - AvoidanceRadius: %.2f"), Movement->AvoidanceConsiderationRadius);
    }
}

void AAIRacer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update racing behavior
    UpdateRacingBehavior(DeltaTime);

    // Check nav mesh every second
    static float LogTimer = 0.0f;
    LogTimer += DeltaTime;
    if (LogTimer >= 1.0f)
    {
        LogTimer = 0.0f;

        UCharacterMovementComponent* Movement = GetCharacterMovement();
        if (Movement)
        {
            // Get current state
            FVector Velocity = Movement->Velocity;
            float Speed = Velocity.Size();
            FVector Location = GetActorLocation();
            FRotator Rotation = GetActorRotation();

            // Check nav mesh position
            UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
            if (NavSystem)
            {
                ANavigationData* NavData = NavSystem->GetDefaultNavDataInstance();
                if (NavData)
                {
                    FNavLocation ProjectedLocation;
                    bool bOnNavMesh = NavSystem->ProjectPointToNavigation(Location, ProjectedLocation, NavData->GetDefaultQueryExtent());
                    float HeightDiff = FMath::Abs(Location.Z - ProjectedLocation.Location.Z);
                    
                    // Log nav mesh status
                    UE_LOG(LogTemp, Warning, TEXT("AIRacer %s Nav Mesh Status:"), *GetName());
                    UE_LOG(LogTemp, Warning, TEXT("  - On Nav Mesh: %s"), bOnNavMesh ? TEXT("Yes") : TEXT("No"));
                    UE_LOG(LogTemp, Warning, TEXT("  - Current Height: %.2f"), Location.Z);
                    UE_LOG(LogTemp, Warning, TEXT("  - Nav Mesh Height: %.2f"), ProjectedLocation.Location.Z);
                    UE_LOG(LogTemp, Warning, TEXT("  - Height Difference: %.2f"), HeightDiff);
                    
                    // Fix height if needed
                    if (!bOnNavMesh || HeightDiff > 100.0f)
                    {
                        UE_LOG(LogTemp, Error, TEXT("  - WARNING: Racer may be off nav mesh or too far above/below!"));
                        
                        if (bOnNavMesh)
                        {
                            // Smoothly move to correct height
                            FVector TargetLocation = GetActorLocation();
                            TargetLocation.Z = ProjectedLocation.Location.Z + 50.0f;
                            
                            FVector NewLocation = FMath::VInterpTo(
                                GetActorLocation(),
                                TargetLocation,
                                DeltaTime,
                                2.0f
                            );
                            
                            SetActorLocation(NewLocation);
                        }
                    }
                }
            }
        }
    }
}

/**
 * Updates the racer's movement behavior each frame
 * Handles speed adjustments, cornering, and collision avoidance
 * @param DeltaTime Time elapsed since last frame
 */
void AAIRacer::UpdateRacingBehavior(float DeltaTime)
{
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (!Movement) return;

    // Get current state
    FVector CurrentVelocity = Movement->Velocity;
    CurrentSpeed = CurrentVelocity.Size();

    // Get AI controller and check waypoint
    AAIRacerContoller* RacerController = Cast<AAIRacerContoller>(GetController());
    if (!RacerController || !RacerController->GetCurrentWaypoint()) return;

    // Calculate path to waypoint
    FVector ToWaypoint = RacerController->GetCurrentWaypoint()->GetActorLocation() - GetActorLocation();
    float DistanceToWaypoint = ToWaypoint.Size();
    FVector DirectionToWaypoint = ToWaypoint.GetSafeNormal();

    // Get turn angle
    float AngleToWaypoint = FMath::RadiansToDegrees(
        FMath::Acos(FVector::DotProduct(GetActorForwardVector(), DirectionToWaypoint)));

    // Calculate target speed
    float DesiredSpeed = CalculateDesiredSpeed(DistanceToWaypoint, AngleToWaypoint);
    
    // Adjust speed
    if (CurrentSpeed < DesiredSpeed)
    {
        // Speed up
        Movement->MaxWalkSpeed = FMath::Min(Movement->MaxWalkSpeed + AccelerationRate * DeltaTime, MaxSpeed);
    }
    else if (CurrentSpeed > DesiredSpeed)
    {
        // Slow down
        Movement->MaxWalkSpeed = FMath::Max(Movement->MaxWalkSpeed - BrakingRate * DeltaTime, MinCorneringSpeed);
    }

    // Handle collision avoidance
    if (Movement->bUseRVOAvoidance)
    {
        TArray<AActor*> NearbyRacers;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAIRacer::StaticClass(), NearbyRacers);
        
        for (AActor* OtherActor : NearbyRacers)
        {
            if (OtherActor != this)
            {
                float Distance = FVector::Distance(GetActorLocation(), OtherActor->GetActorLocation());
                if (Distance < Movement->AvoidanceConsiderationRadius)
                {
                    // Increase avoidance when near others
                    Movement->AvoidanceWeight = FMath::Min(Movement->AvoidanceWeight + DeltaTime, 1.0f);
                    break;
                }
            }
        }
    }
}

/**
 * Adjusts the racer's speed based on corner angle
 * Sharper corners result in more speed reduction
 * @param CornerAngle Angle of the upcoming corner in degrees
 */
void AAIRacer::AdjustSpeedForCorner(float CornerAngle)
{
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (!Movement) return;

    // Calculate speed reduction based on corner sharpness
    float AngleRatio = FMath::Clamp(CornerAngle / MaxCorneringAngle, 0.0f, 1.0f);
    float SpeedMultiplier = FMath::Lerp(1.0f, CorneringSpeedMultiplier, AngleRatio);
    
    // Apply speed limit while maintaining minimum
    Movement->MaxWalkSpeed = FMath::Max(MaxSpeed * SpeedMultiplier, MinCorneringSpeed);
}

/**
 * Calculates the optimal speed based on corner angle and distance
 * Takes into account both the sharpness of the turn and distance to the corner
 * @param DistanceToCorner Distance to the next corner
 * @param CornerAngle Angle of the upcoming corner in degrees
 * @return The calculated desired speed
 */
float AAIRacer::CalculateDesiredSpeed(float DistanceToCorner, float CornerAngle)
{
    float DesiredSpeed = MaxSpeed;

    // Reduce speed for sharp turns
    if (CornerAngle > 0.0f)
    {
        float AngleRatio = FMath::Clamp(CornerAngle / MaxCorneringAngle, 0.0f, 1.0f);
        DesiredSpeed *= FMath::Lerp(1.0f, CorneringSpeedMultiplier, AngleRatio);
    }

    // Brake when approaching corner
    if (DistanceToCorner < BrakingDistance)
    {
        float DistanceRatio = FMath::Clamp(DistanceToCorner / BrakingDistance, 0.0f, 1.0f);
        DesiredSpeed = FMath::Lerp(MinCorneringSpeed, DesiredSpeed, DistanceRatio);
    }

    return FMath::Max(DesiredSpeed, MinCorneringSpeed);
}