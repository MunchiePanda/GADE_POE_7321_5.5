#pragma once

#include "CoreMinimal.h"
#include "SpectatorState.generated.h"

class ASpectator;

UINTERFACE(MinimalAPI)
class USpectatorState : public UInterface
{
    GENERATED_BODY()
};

class ISpectatorState
{
    GENERATED_BODY()

public:
    virtual void EnterState(ASpectator* Spectator) = 0;
    virtual void UpdateState(ASpectator* Spectator, float DeltaTime) = 0;
    virtual void ExitState(ASpectator* Spectator) = 0;
    virtual FName GetStateName() const = 0;
};