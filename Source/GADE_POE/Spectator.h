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

	//Set the current state of the spectator
    UFUNCTION(BlueprintCallable, Category = "Spectator") 
    void SetState(TScriptInterface<ISpectatorState> NewState);

	//Set the current state of the spectator to cheering
    UFUNCTION(BlueprintCallable, Category = "Spectator")
    void Cheer();

	//Set the current state of the spectator to disappointed
    UFUNCTION(BlueprintCallable, Category = "Spectator")
    void BeDisappointed();

	//Set the current state of the spectator to idle
    UFUNCTION(BlueprintCallable, Category = "Spectator")
    void BeIdle();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spectator")
	class USkeletalMeshComponent* SpectatorMesh; // Mesh component for the spectator

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spectator")
	TScriptInterface<ISpectatorState> CurrentState; // Current state of the spectator

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spectator")
	FName CurrentStateName; // Name of the current state

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spectator Animation")
	FName IdleAnimationName = "Idle"; // Animation name for idle state

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spectator Animation")
	FName CheeringAnimationName = "Cheering"; // Animation name for cheering state

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spectator Animation")
	FName DisappointedAnimationName = "Disappointed"; // Animation name for disappointed state

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spectator")
	float ReactionDistance = 1000.0f; // Distance within which the spectator reacts to racers


	float StateTimer; // Timer for the current state
};