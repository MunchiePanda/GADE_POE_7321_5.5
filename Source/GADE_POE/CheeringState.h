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
    virtual void EnterState(ASpectator* Spectator) override;
    virtual void UpdateState(ASpectator* Spectator, float DeltaTime) override;
    virtual void ExitState(ASpectator* Spectator) override;
    virtual FName GetStateName() const override { return FName("Cheering"); }

private:
    float CheerDuration = 5.0f;
};