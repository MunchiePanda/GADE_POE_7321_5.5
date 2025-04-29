#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SpectatorState.h"
#include "IdleState.generated.h"

UCLASS()
class GADE_POE_API UIdleState : public UObject, public ISpectatorState
{
    GENERATED_BODY()

public:
	virtual void EnterState(ASpectator* Spectator) override; // Called when the spectator enters the state
	virtual void UpdateState(ASpectator* Spectator, float DeltaTime) override; // Called every frame while the spectator is in the state
	virtual void ExitState(ASpectator* Spectator) override; // Called when the spectator exits the state
	virtual FName GetStateName() const override { return FName("Idle"); } //	Returns the name of the state
};