#include "AIRacer.h"
#include "Components/StaticMeshComponent.h"

AAIRacer::AAIRacer() // Constructor
{
    PrimaryActorTick.bCanEverTick = false;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;
    Mesh->SetCollisionProfileName(TEXT("Pawn"));
    MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));

    // Default attributes
    RacerType = ERacerType::Medium;
    MaxSpeed = 600.0f;
    Acceleration = 500.0f;
}

void AAIRacer::BeginPlay()
{
    Super::BeginPlay();
    SetupRacerAttributes();
}

// Sets the racer's attributes based on the racer type
void AAIRacer::SetupRacerAttributes()
{
    switch (RacerType)
    {
    case ERacerType::Fast:
        MaxSpeed = 1200.0f;
        Acceleration = 900.0f;
        break;
    case ERacerType::Medium:
        MaxSpeed = 600.0f;
        Acceleration = 500.0f;
        break;
    case ERacerType::Slow:
        MaxSpeed = 300.0f;
        Acceleration = 200.0f;
        break;
    }

    // Update movement component attributes
    if (MovementComponent)
    {
        MovementComponent->MaxSpeed = MaxSpeed;
        MovementComponent->Acceleration = Acceleration;
    }
}