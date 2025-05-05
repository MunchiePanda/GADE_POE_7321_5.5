#include "Spectator.h"
#include "IdleState.h"
#include "CheeringState.h"
#include "DisappointedState.h"

ASpectator::ASpectator()
{
    PrimaryActorTick.bCanEverTick = true;

    SpectatorMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SpectatorMesh"));
    RootComponent = SpectatorMesh;

    // Randomize initial delay between 0 and 10 seconds
    InitialStateDelay = FMath::RandRange(0.0f, 10.0f);
}

void ASpectator::BeginPlay()
{
    Super::BeginPlay();

    // Randomly select initial state
    int32 RandomState = FMath::RandRange(0, 2);
    switch (RandomState)
    {
    case 0:
		BeIdle(); // Set the initial state to Idle
        UE_LOG(LogTemp, Log, TEXT("Spectator %s starting in Idle state"), *GetName());
        break;
    case 1:
        Cheer(); // Set the initial state to Cheer
        UE_LOG(LogTemp, Log, TEXT("Spectator %s starting in Cheering state"), *GetName());
        break;
    case 2:
        BeDisappointed(); // Set the initial state to Disappointed
        UE_LOG(LogTemp, Log, TEXT("Spectator %s starting in Disappointed state"), *GetName());
        break;
    default:
        BeIdle(); // Fallback
        UE_LOG(LogTemp, Warning, TEXT("Spectator %s fallback to Idle state"), *GetName());
        break;
    }

    // Start the StateSwitchTimer immediately
    GetWorldTimerManager().SetTimer(StateSwitchTimer, this, &ASpectator::SwitchToNextState, 20.0f, true);
}

void ASpectator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime); 
    if (CurrentState)
    {
        CurrentState->UpdateState(this, DeltaTime); // Update the current state
    }
}

void ASpectator::SetState(TScriptInterface<ISpectatorState> NewState)
{
    if (CurrentState)
    {
        CurrentState->ExitState(this); // Exit the current state
    }

    CurrentState = NewState;

    if (CurrentState)
    {
        CurrentState->EnterState(this);
        CurrentStateName = CurrentState->GetStateName(); // Set the current state name
    }
}

void ASpectator::Cheer()
{ 
    SetState(NewObject<UCheeringState>(this)); // Set the cheering state
}

void ASpectator::BeDisappointed()
{
	SetState(NewObject<UDisappointedState>(this)); // Set the disappointed state
}

void ASpectator::BeIdle()
{
	SetState(NewObject<UIdleState>(this)); //  Set the idle state
}

void ASpectator::SwitchToNextState()
{
    UE_LOG(LogTemp, Warning, TEXT("Switching from: %s"), *CurrentStateName.ToString());

    // Switch to the next state based on the current state
    if (CurrentStateName == "Idle")
    {
        Cheer();
    }
    else if (CurrentStateName == "Cheering")
    {
        BeDisappointed();
    }
    else if (CurrentStateName == "Disappointed")
    {
        BeIdle();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Unexpected state: %s"), *CurrentStateName.ToString());
        BeIdle(); // Fallback to Idle for safety
    }
}
