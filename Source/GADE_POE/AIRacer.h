#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "AIRacer.generated.h"

UCLASS()
class GADE_POE_API AAIRacer : public APawn
{
    GENERATED_BODY()

public:
    AAIRacer();

    UPROPERTY(VisibleAnywhere)
    UFloatingPawnMovement* MovementComponent;

    UPROPERTY(VisibleAnywhere, Category = "Mesh")
    UStaticMeshComponent* Mesh;

    UPROPERTY(BlueprintReadOnly, Category = "Race")
    int32 LapCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Race")
    int32 WaypointsPassed = 0;
};
