#include "Spectator.h"
#include "IdleState.h"
#include "CheeringState.h"
#include "DisappointedState.h"

// Sets default values
ASpectator::ASpectator()
{
    PrimaryActorTick.bCanEverTick = true;

    SpectatorMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SpectatorMesh"));
    RootComponent = SpectatorMesh;

    StateTimer = 0.0f;
}

void ASpectator::BeginPlay()
{
    Super::BeginPlay();
	BeIdle(); // Start in idle state
}

void ASpectator::Tick(float DeltaTime) 
{
	Super::Tick(DeltaTime); // Call the base class tick function

    if (CurrentState)
    {
        CurrentState->UpdateState(this, DeltaTime);
		StateTimer += DeltaTime; // Update the state timer
    }
}

void ASpectator::SetState(TScriptInterface<ISpectatorState> NewState) // Set the current state of the spectator
{
	if (CurrentState) // Check if there is a current state
    {
        CurrentState->ExitState(this);
    }

    CurrentState = NewState;

	if (CurrentState) // Check if the new state is valid
    {
        CurrentState->EnterState(this);
        CurrentStateName = CurrentState->GetStateName();
        StateTimer = 0.0f;
    }
}

void ASpectator::Cheer() // Set the current state of the spectator to cheering
{
    SetState(NewObject<UCheeringState>(this));
}

void ASpectator::BeDisappointed() // Set the current state of the spectator to disappointed
{
    SetState(NewObject<UDisappointedState>(this));
}

void ASpectator::BeIdle() // Set the current state of the spectator to idle
{
    SetState(NewObject<UIdleState>(this));
}