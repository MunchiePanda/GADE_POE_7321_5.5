#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AIRacer.generated.h"

class AWaypointManager;
class UNavigationSystemV1;

UCLASS()
class GADE_POE_API AAIRacer : public APawn
{
    GENERATED_BODY()

public:
    AAIRacer();

protected:
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UStaticMeshComponent* Mesh;

    UPROPERTY(EditAnywhere, Category = "AI")
    float MoveSpeed;

    UPROPERTY(VisibleAnywhere, Category = "AI")
    int32 CurrentWaypointIndex;

    UPROPERTY(VisibleAnywhere, Category = "AI")
    AWaypointManager* WaypointManager;

    UPROPERTY(Transient)
    UNavigationSystemV1* NavSystem;

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

private:
    void MoveToNextWaypoint();
};