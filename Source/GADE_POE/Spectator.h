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
	void SetState(TScriptInterface<ISpectatorState> NewState); // Function to set the current state of the spectator

    UFUNCTION(BlueprintCallable, Category = "Spectator")
	void Cheer(); // Cheer function to set the spectator to cheering state

    UFUNCTION(BlueprintCallable, Category = "Spectator")
	void BeDisappointed(); // Function to set the spectator to disappointed state

    UFUNCTION(BlueprintCallable, Category = "Spectator")
	void BeIdle(); // Function to set the spectator to idle state

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spectator")
    class USkeletalMeshComponent* SpectatorMesh; // The skeletal mesh component of the spectator

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spectator")
	TScriptInterface<ISpectatorState> CurrentState; // The current state of the spectator 

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spectator")
    FName CurrentStateName; // The name of the current state

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spectator Animation")
	FName IdleAnimationName = "Idle"; // The name of the idle animation

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spectator Animation")
    FName CheeringAnimationName = "Cheering"; // The name of the cheering animation

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spectator Animation")
    FName DisappointedAnimationName = "Disappointed"; // The name of the disappointed animation

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spectator")
    float ReactionDistance = 1000.0f; // The distance at which the spectator reacts

    FTimerHandle StateSwitchTimer;
    void SwitchToNextState(); // Function to switch to the next state

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spectator")
    float InitialStateDelay = 0.0f; // Initial delay before starting state transitions (in seconds)
};

