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
	virtual void EnterState(ASpectator* Spectator) override; // Called when the spectator enters the disappointed state
	virtual void UpdateState(ASpectator* Spectator, float DeltaTime) override; // Called every frame while the spectator is in the disappointed state
	virtual void ExitState(ASpectator* Spectator) override; // Called when the spectator exits the disappointed state
	virtual FName GetStateName() const override { return FName("Disappointed"); } // Returns the name of the disappointed state

private:
	float DisappointedDuration = 5.0f; // Duration of the disappointed state
};