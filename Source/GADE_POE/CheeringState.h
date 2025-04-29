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
	virtual void EnterState(ASpectator* Spectator) override; // Called when the spectator enters the cheering state
	virtual void UpdateState(ASpectator* Spectator, float DeltaTime) override; // Called every frame while the spectator is in the cheering state
	virtual void ExitState(ASpectator* Spectator) override; // Called when the spectator exits the cheering state
	virtual FName GetStateName() const override { return FName("Cheering"); } // Returns the name of the cheering state	

private:
	float CheerDuration = 5.0f; // Duration of the cheering state
};