#include "Spectator.h"
#include "IdleState.h"
#include "CheeringState.h"
#include "DisappointedState.h"

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
    BeIdle();
}

void ASpectator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (CurrentState)
    {
        CurrentState->UpdateState(this, DeltaTime);
        StateTimer += DeltaTime;
    }
}

void ASpectator::SetState(TScriptInterface<ISpectatorState> NewState)
{
    if (CurrentState)
    {
        CurrentState->ExitState(this);
    }

    CurrentState = NewState;

    if (CurrentState)
    {
        CurrentState->EnterState(this);
        CurrentStateName = CurrentState->GetStateName();
        StateTimer = 0.0f;
    }
}

void ASpectator::Cheer()
{
    SetState(NewObject<UCheeringState>(this));
}

void ASpectator::BeDisappointed()
{
    SetState(NewObject<UDisappointedState>(this));
}

void ASpectator::BeIdle()
{
    SetState(NewObject<UIdleState>(this));
}