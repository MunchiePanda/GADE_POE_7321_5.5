#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SpectatorState.h"
#include "CheeringState.generated.h"

UCLASS()
class GADE_POE_API UCheeringState : public UObject, public ISpectatorState
{
    GENERATED_BODY()

public:
	virtual void EnterState(ASpectator* Spectator) override; // Called when entering the cheering state
    virtual void UpdateState(ASpectator* Spectator, float DeltaTime) override; // Called every frame
    virtual void ExitState(ASpectator* Spectator) override; // Called when exiting the cheering state
    virtual FName GetStateName() const override { return FName("Cheering"); } // Returns the name of the cheering state

private:
    float CheerDuration = 5.0f; // Duration of the cheering state
    float StateTimer = 0.0f;    // Timer for this state instance
};