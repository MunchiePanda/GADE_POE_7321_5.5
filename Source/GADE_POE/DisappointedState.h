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
    virtual void EnterState(ASpectator* Spectator) override;
    virtual void UpdateState(ASpectator* Spectator, float DeltaTime) override;
    virtual void ExitState(ASpectator* Spectator) override;
    virtual FName GetStateName() const override { return FName("Disappointed"); }

private:
    float DisappointedDuration = 5.0f;
};