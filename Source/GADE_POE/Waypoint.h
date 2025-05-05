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
    class USphereComponent* TriggerSphere; // Trigger sphere

    UPROPERTY(VisibleAnywhere, Category = "Components")
	class UStaticMeshComponent* VisualMesh; // Visual mesh

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, // Called when the trigger sphere overlaps with another actor
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);
};