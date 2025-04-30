#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RacerTypes.h"
#include "Components/SkeletalMeshComponent.h"
#include "AIRacer.generated.h"

class AAIRacerContoller; // Forward declaration

UCLASS()
class GADE_POE_API AAIRacer : public ACharacter
{
    GENERATED_BODY()

public:
    AAIRacer();

    UPROPERTY(VisibleAnywhere, Category = "Mesh")
    USkeletalMeshComponent* RacerMesh;

    UPROPERTY(BlueprintReadOnly, Category = "Race")
    int32 LapCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Race")
    int32 WaypointsPassed = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    ERacerType RacerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    float MaxSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    float MaxAcceleration;

    void SetupRacerAttributes();

protected:
    virtual void BeginPlay() override;
};