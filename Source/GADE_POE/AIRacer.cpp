#include "AIRacer.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "RacerTypes.h"
#include "Components/SkeletalMeshComponent.h"
#include "AIRacerContoller.h"
#include "Kismet/GameplayStatics.h"
#include "BiginnerRaceGameState.h"

AAIRacer::AAIRacer() // Constructor
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
    if (Movement) // Check if the movement component is valid
    {
        Movement->MaxWalkSpeed = MaxSpeed;
        Movement->MaxAcceleration = MaxAcceleration;
    }
}