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
        Movement->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
        Movement->bUseControllerDesiredRotation = true;
        
        // Ground movement settings
        Movement->NavAgentProps.bCanWalk = true;
        Movement->NavAgentProps.bCanFly = false;
        Movement->bCanWalkOffLedges = false;
        Movement->SetMovementMode(MOVE_Walking);
        Movement->GravityScale = 8.0f; // Significantly increased gravity
        
        // Adjust movement speeds
        Movement->MaxWalkSpeed = 600.0f;
        Movement->MaxAcceleration = 2000.0f;
        Movement->BrakingDecelerationWalking = 2000.0f;
        Movement->GroundFriction = 8.0f;
        
        // Navigation settings
        Movement->NavAgentProps.AgentRadius = 40.0f;
        Movement->NavAgentProps.AgentHeight = 96.0f;
        Movement->bUseRVOAvoidance = true;
        Movement->AvoidanceConsiderationRadius = 100.0f;
        
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
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AIRacer: Failed to find BeginnerRaceGameState."));
    }
}

void AAIRacer::SetupRacerAttributes()
{
    switch (RacerType)
    {
    case ERacerType::Fast:
        MaxSpeed = 4000.0f; // Reduced for testing
        MaxAcceleration = 900.0f;
        break;
    case ERacerType::Medium:
        MaxSpeed = 4000.0f;
        MaxAcceleration = 500.0f;
        break;
    case ERacerType::Slow:
        MaxSpeed = 4000.0f;
        MaxAcceleration = 300.0f;
        break;
    }

    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (Movement)
    {
        Movement->MaxWalkSpeed = MaxSpeed;
        Movement->MaxAcceleration = MaxAcceleration;
    }
}