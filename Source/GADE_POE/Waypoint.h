#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Waypoint.generated.h"

UCLASS()
class GADE_POE_API AWaypoint : public AActor
{
    GENERATED_BODY()

public:
    AWaypoint();

protected:
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class USphereComponent* TriggerSphere;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UStaticMeshComponent* VisualMesh;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);
};