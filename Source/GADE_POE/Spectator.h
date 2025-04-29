#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpectatorState.h"
#include "Spectator.generated.h"

UCLASS()
class GADE_POE_API ASpectator : public AActor
{
    GENERATED_BODY()

public:
    ASpectator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Spectator")
    void SetState(TScriptInterface<ISpectatorState> NewState);

    UFUNCTION(BlueprintCallable, Category = "Spectator")
    void Cheer();

    UFUNCTION(BlueprintCallable, Category = "Spectator")
    void BeDisappointed();

    UFUNCTION(BlueprintCallable, Category = "Spectator")
    void BeIdle();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spectator")
    class USkeletalMeshComponent* SpectatorMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spectator")
    TScriptInterface<ISpectatorState> CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spectator")
    FName CurrentStateName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spectator Animation")
    FName IdleAnimationName = "Idle";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spectator Animation")
    FName CheeringAnimationName = "Cheering";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spectator Animation")
    FName DisappointedAnimationName = "Disappointed";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spectator")
    float ReactionDistance = 1000.0f;


    float StateTimer;
};