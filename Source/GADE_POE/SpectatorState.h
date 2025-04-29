#pragma once

#include "CoreMinimal.h"
#include "SpectatorState.generated.h"

class ASpectator; // Forward declaration of ASpectator class

UINTERFACE(MinimalAPI) // This is the interface for the spectator state
class USpectatorState : public UInterface 
{
    GENERATED_BODY()
};

class ISpectatorState
{
    GENERATED_BODY()

public:
	virtual void EnterState(ASpectator* Spectator) = 0;  // Called when the spectator enters the state
	virtual void UpdateState(ASpectator* Spectator, float DeltaTime) = 0; // Called every frame while the spectator is in the state
	virtual void ExitState(ASpectator* Spectator) = 0; // Called when the spectator exits the state
	virtual FName GetStateName() const = 0; // Returns the name of the state
};