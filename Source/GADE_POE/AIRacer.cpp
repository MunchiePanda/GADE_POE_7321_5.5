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

    UCapsuleComponent* Capsule = GetCapsuleComponent();
    if (Capsule)
    {
        Capsule->SetCapsuleSize(40.0f, 96.0f);
        Capsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly); // Only use for overlap queries
        Capsule->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
        Capsule->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
        Capsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Overlap);
        Capsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
        Capsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    }

    RacerMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RacerMesh"));
    RacerMesh->SetupAttachment(RootComponent);
    RacerMesh->SetCollisionProfileName(TEXT("NoCollision"));

    PhysicsBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PhysicsBody"));
    PhysicsBody->SetupAttachment(RootComponent);
    PhysicsBody->SetSimulatePhysics(true);
    PhysicsBody->SetEnableGravity(false); // Initially disabled, enabled after grounding
    PhysicsBody->SetConstraintMode(EDOFMode::Default);
    PhysicsBody->SetAngularDamping(0.1f); // Lowered for smoother rotation
    PhysicsBody->SetLinearDamping(0.05f); // Lowered for smoother movement
    PhysicsBody->SetCollisionProfileName(TEXT("Pawn"));
    PhysicsBody->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    PhysicsBody->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (Movement)
    {
        Movement->bOrientRotationToMovement = false;
        Movement->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
        Movement->bUseControllerDesiredRotation = false;
        Movement->NavAgentProps.bCanWalk = true;
        Movement->bCanWalkOffLedges = false;
        Movement->SetMovementMode(MOVE_Flying);
        Movement->GravityScale = 0.0f;
    }

    AIControllerClass = AAIRacerContoller::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    RacerType = ERacerType::Medium;
    MaxSpeed = 600.0f;
    MaxAcceleration = 500.0f;
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

    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (Movement)
    {
        Movement->MaxWalkSpeed = MaxSpeed;
        Movement->MaxAcceleration = MaxAcceleration;
    }
}