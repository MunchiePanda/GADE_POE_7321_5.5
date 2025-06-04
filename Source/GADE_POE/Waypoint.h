#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h" // Added for USphereComponent
#include "Components/StaticMeshComponent.h" // Added for UStaticMeshComponent
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

    virtual void BeginPlay() override;
    virtual void BeginDestroy() override;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);
};