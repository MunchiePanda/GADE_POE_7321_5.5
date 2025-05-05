#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RacerTypes.h"
#include "Components/SkeletalMeshComponent.h"
#include "AIRacer.generated.h"

class AAIRacerContoller; // Forward declaration
class ABeginnerRaceGameState; // Forward declaration

UCLASS()
class GADE_POE_API AAIRacer : public ACharacter
{
    GENERATED_BODY()

public:
    AAIRacer();
    // Sets default values for this character's properties
    UPROPERTY(VisibleAnywhere, Category = "Mesh")
    USkeletalMeshComponent* RacerMesh; // Reference to the skeletal mesh component

    UPROPERTY(BlueprintReadOnly, Category = "Race")
    int32 LapCount = 0; // Number of laps completed

    UPROPERTY(BlueprintReadOnly, Category = "Race")
    int32 WaypointsPassed = 0; // Number of waypoints passed

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    ERacerType RacerType; // Type of racer

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
	float MaxSpeed; // Maximum speed of the racer

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
	float MaxAcceleration; // Maximum acceleration of the racer

	void SetupRacerAttributes(); // Set up racer attributes based on type of racer

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    ABeginnerRaceGameState* GameState; // Reference to the game state
};