#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SpectatorState.h"
#include "DisappointedState.generated.h"

UCLASS()
class GADE_POE_API UDisappointedState : public UObject, public ISpectatorState
{
    GENERATED_BODY()

public:
    virtual void EnterState(ASpectator* Spectator) override; // Called when entering the Disappointed state
    virtual void UpdateState(ASpectator* Spectator, float DeltaTime) override; // Update the state
    virtual void ExitState(ASpectator* Spectator) override; // Called when exiting the Disappointed state
	virtual FName GetStateName() const override { return FName("Disappointed"); } // Returns the name of the Disappointed state

private:
    float DisappointedDuration = 5.0f; // Duration of the disappointed state
    float StateTimer = 0.0f;           // Timer for this state instance
};