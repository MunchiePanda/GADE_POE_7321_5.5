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
        Capsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
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
    PhysicsBody->SetSimulatePhysics(false); // Disable physics for navigation
    PhysicsBody->SetEnableGravity(false);
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
        Movement->NavAgentProps.bCanFly = true; // Enable flying for MOVE_Flying
        Movement->bCanWalkOffLedges = false;
        Movement->SetMovementMode(MOVE_Flying);
        Movement->GravityScale = 0.0f;
        Movement->MaxFlySpeed = 600.0f; // Explicitly set for flying
        Movement->MaxAcceleration = 500.0f;
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
        MaxSpeed = 4000.0f; // Reduced for testing
        MaxAcceleration = 900.0f;
        break;
    case ERacerType::Medium:
        MaxSpeed = 600.0f;
        MaxAcceleration = 4000.0f;
        break;
    case ERacerType::Slow:
        MaxSpeed = 4000.0f;
        MaxAcceleration = 300.0f;
        break;
    }

    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (Movement)
    {
        Movement->MaxFlySpeed = MaxSpeed;
        Movement->MaxAcceleration = MaxAcceleration;
    }
}