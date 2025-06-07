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

    // Configure the capsule component (already the root component from ACharacter)
    UCapsuleComponent* Capsule = GetCapsuleComponent();
    if (Capsule)
    {
        Capsule->SetCapsuleSize(40.0f, 96.0f);
        Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Capsule->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
        Capsule->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
        Capsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
        Capsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Block);
        Capsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    }

    // Set up the skeletal mesh as a child of the capsule
    RacerMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RacerMesh"));
    RacerMesh->SetupAttachment(RootComponent);
    RacerMesh->SetCollisionProfileName(TEXT("NoCollision"));

    // Configure the character movement component
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (Movement)
    {
        Movement->bOrientRotationToMovement = true;
        Movement->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
        Movement->bUseControllerDesiredRotation = false;
        Movement->NavAgentProps.bCanWalk = true;
        Movement->bCanWalkOffLedges = false;
    }

    // Set the AI controller class
    AIControllerClass = AAIRacerContoller::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    // Default attributes
    RacerType = ERacerType::Medium;
    MaxSpeed = 600.0f;
    MaxAcceleration = 500.0f;
}

void AAIRacer::BeginPlay()
{
    Super::BeginPlay();
    SetupRacerAttributes();

    // Register with GameState
    GameState = Cast<ABeginnerRaceGameState>(GetWorld()->GetGameState());
    if (GameState)
    {
        GameState->RegisterRacer(this);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AIRacer: Failed to find BeginnerRaceGameState."));
    }
}

void AAIRacer::SetupRacerAttributes()
{
    // Calculate attributes based on RacerType
    switch (RacerType)
    {
    case ERacerType::Fast:
        MaxSpeed = 4000.0f;
        MaxAcceleration = 900.0f;
        break;
    case ERacerType::Medium:
        MaxSpeed = 4000.0f;
        MaxAcceleration = 500.0f;
        break;
    case ERacerType::Slow:
        MaxSpeed = 4000.0f;
        MaxAcceleration = 50.0f;
        break;
    }

    // Update movement component attributes
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (Movement)
    {
        Movement->MaxWalkSpeed = MaxSpeed;
        Movement->MaxAcceleration = MaxAcceleration;
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