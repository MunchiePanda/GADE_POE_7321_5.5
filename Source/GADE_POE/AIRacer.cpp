#include "AIRacer.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "RacerTypes.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "AIRacerContoller.h"
#include "Kismet/GameplayStatics.h"
#include "BiginnerRaceGameState.h"

AAIRacer::AAIRacer()
{
    PrimaryActorTick.bCanEverTick = true;

    // Setup capsule collision
    UCapsuleComponent* Capsule = GetCapsuleComponent();
    if (Capsule)
    {
        Capsule->SetCapsuleSize(40.0f, 96.0f);
        Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Capsule->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
        // Set up collision responses
        FCollisionResponseContainer ResponseContainer;
        ResponseContainer.SetAllChannels(ECR_Block);
        ResponseContainer.SetResponse(ECC_Pawn, ECR_Overlap);
        ResponseContainer.SetResponse(ECC_Camera, ECR_Ignore);
        Capsule->SetCollisionResponseToChannels(ResponseContainer);
        
        // Enable hit events
        Capsule->SetNotifyRigidBodyCollision(true);
        Capsule->SetGenerateOverlapEvents(true);
    }

    // Setup skeletal mesh
    RacerMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RacerMesh"));
    RacerMesh->SetupAttachment(RootComponent);
    RacerMesh->SetCollisionProfileName(TEXT("NoCollision"));

    // Setup physics body
    PhysicsBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PhysicsBody"));
    PhysicsBody->SetupAttachment(RootComponent);
    PhysicsBody->SetSimulatePhysics(false);
    PhysicsBody->SetEnableGravity(true);
    PhysicsBody->SetCollisionProfileName(TEXT("BlockAllDynamic"));

    // Setup movement component
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (Movement)
    {
        Movement->bOrientRotationToMovement = true;
        Movement->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
        Movement->bUseControllerDesiredRotation = true;
        
        // Ground movement settings
        Movement->NavAgentProps.bCanWalk = true;
        Movement->NavAgentProps.bCanFly = false;
        Movement->bCanWalkOffLedges = false;
        Movement->SetMovementMode(MOVE_Walking);
        Movement->GravityScale = 8.0f;
        
        // Adjust movement speeds
        Movement->MaxWalkSpeed = 600.0f;
        Movement->MaxAcceleration = 2000.0f;
        Movement->BrakingDecelerationWalking = 4000.0f;
        Movement->GroundFriction = 12.0f;
        
        // Navigation settings
        Movement->NavAgentProps.AgentRadius = 50.0f;
        Movement->NavAgentProps.AgentHeight = 96.0f;
        Movement->bUseRVOAvoidance = true;
        Movement->AvoidanceConsiderationRadius = 200.0f;
        Movement->AvoidanceWeight = 1.0f;
        
        // Physics settings
        Movement->Mass = 100.0f;
        Movement->bMaintainHorizontalGroundVelocity = true;
        Movement->bSnapToPlaneAtStart = true;
        Movement->SetWalkableFloorAngle(50.0f);
        
        // Additional stability settings
        Movement->bAlwaysCheckFloor = true;
        Movement->bUseFlatBaseForFloorChecks = true;
        Movement->MinAnalogWalkSpeed = 0.0f;
        Movement->bEnablePhysicsInteraction = true;

        // New acceleration settings
        Movement->MaxCustomMovementSpeed = 800.0f;
        Movement->bRequestedMoveUseAcceleration = true;
    }

    AIControllerClass = AAIRacerContoller::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    RacerType = ERacerType::Medium;
    MaxSpeed = 600.0f;
    MaxAcceleration = 2000.0f;
}

void AAIRacer::BeginPlay()
{
    Super::BeginPlay();
    SetupRacerAttributes();

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

    // Log initial configuration
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

    float BaseSpeed = 4000.0f;
    float BaseAccel = 2000.0f;
    float AvoidanceRadius = 200.0f;

    UE_LOG(LogTemp, Log, TEXT("AIRacer %s: Setting up attributes"), *GetName());

    switch (RacerType)
    {
    case ERacerType::Fast:
        MaxSpeed = BaseSpeed * 1.2f;
        MaxAcceleration = BaseAccel * 1.2f;
        if (Movement)
        {
            Movement->AvoidanceConsiderationRadius = AvoidanceRadius * 1.5f;
            Movement->RotationRate = FRotator(0.0f, 900.0f, 0.0f);
            Movement->BrakingDecelerationWalking = 6000.0f;
            UE_LOG(LogTemp, Log, TEXT("AIRacer %s: Configured as Fast type"), *GetName());
        }
        break;
    case ERacerType::Medium:
        MaxSpeed = BaseSpeed;
        MaxAcceleration = BaseAccel;
        if (Movement)
        {
            Movement->AvoidanceConsiderationRadius = AvoidanceRadius * 1.2f;
            Movement->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
            Movement->BrakingDecelerationWalking = 4000.0f;
            UE_LOG(LogTemp, Log, TEXT("AIRacer %s: Configured as Medium type"), *GetName());
        }
        break;
    case ERacerType::Slow:
        MaxSpeed = BaseSpeed * 0.8f;
        MaxAcceleration = BaseAccel * 0.8f;
        if (Movement)
        {
            Movement->AvoidanceConsiderationRadius = AvoidanceRadius;
            Movement->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
            Movement->BrakingDecelerationWalking = 3000.0f;
            UE_LOG(LogTemp, Log, TEXT("AIRacer %s: Configured as Slow type"), *GetName());
        }
        break;
    }

    if (Movement)
    {
        Movement->MaxWalkSpeed = MaxSpeed;
        Movement->MaxAcceleration = MaxAcceleration;
        Movement->MaxCustomMovementSpeed = MaxSpeed * 1.2f;
        Movement->AvoidanceWeight = 1.0f;

        // Log the final configuration
        UE_LOG(LogTemp, Log, TEXT("AIRacer %s: Final Movement Configuration:"), *GetName());
        UE_LOG(LogTemp, Log, TEXT("  - MaxWalkSpeed: %.2f"), Movement->MaxWalkSpeed);
        UE_LOG(LogTemp, Log, TEXT("  - MaxAcceleration: %.2f"), Movement->MaxAcceleration);
        UE_LOG(LogTemp, Log, TEXT("  - MaxCustomSpeed: %.2f"), Movement->MaxCustomMovementSpeed);
        UE_LOG(LogTemp, Log, TEXT("  - AvoidanceRadius: %.2f"), Movement->AvoidanceConsiderationRadius);
    }
}

// Add a new Tick function to monitor movement
void AAIRacer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Log movement data every second (using a timer to avoid spam)
    static float LogTimer = 0.0f;
    LogTimer += DeltaTime;
    if (LogTimer >= 1.0f)
    {
        LogTimer = 0.0f;

        UCharacterMovementComponent* Movement = GetCharacterMovement();
        if (Movement)
        {
            FVector Velocity = Movement->Velocity;
            float Speed = Velocity.Size();
            FVector Location = GetActorLocation();
            FRotator Rotation = GetActorRotation();

            UE_LOG(LogTemp, Verbose, TEXT("AIRacer %s Status:"), *GetName());
            UE_LOG(LogTemp, Verbose, TEXT("  - Location: %s"), *Location.ToString());
            UE_LOG(LogTemp, Verbose, TEXT("  - Rotation: %s"), *Rotation.ToString());
            UE_LOG(LogTemp, Verbose, TEXT("  - Speed: %.2f (%.1f%% of max)"), Speed, (Speed / Movement->MaxWalkSpeed) * 100.0f);
            UE_LOG(LogTemp, Verbose, TEXT("  - Velocity: %s"), *Velocity.ToString());

            // Log if we're in avoidance
            if (Movement->bUseRVOAvoidance)
            {
                UE_LOG(LogTemp, Verbose, TEXT("  - Using RVO Avoidance"));
                UE_LOG(LogTemp, Verbose, TEXT("  - Avoidance Weight: %.2f"), Movement->AvoidanceWeight);
            }
        }
    }
}