#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "RacerTypes.h"
#include "AIRacer.generated.h"

UCLASS()
class GADE_POE_API AAIRacer : public APawn
{
    GENERATED_BODY()

public:
    AAIRacer();

    UPROPERTY(VisibleAnywhere)
    UFloatingPawnMovement* MovementComponent; // Reference to the movement component

    UPROPERTY(VisibleAnywhere, Category = "Mesh")
	UStaticMeshComponent* Mesh; // Reference to the mesh component

    UPROPERTY(BlueprintReadOnly, Category = "Race")
    int32 LapCount = 0; // Number of laps completed

    UPROPERTY(BlueprintReadOnly, Category = "Race")
	int32 WaypointsPassed = 0; // Number of waypoints passed

    // Racer type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    ERacerType RacerType; // Racer type

    // Attributes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
	float MaxSpeed; // Maximum speed of the racer

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    float Acceleration;
  // Set attributes based on racer type
    void SetupRacerAttributes();
protected:
    virtual void BeginPlay() override; 

  
};