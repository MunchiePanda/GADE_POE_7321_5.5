// BarrierSplineActor.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "BarrierSplineActor.generated.h"

/**
 * Actor that creates a barrier along a spline path
 */
UCLASS(Blueprintable)
class GADE_POE_API ABarrierSplineActor : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ABarrierSplineActor();

    // Called after the object is loaded
    virtual void PostLoad() override;

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrier|Spline")
    class USplineComponent* Spline;

    UPROPERTY(EditAnywhere, Category = "Barrier|Mesh")
    class UStaticMesh* BarrierMesh;

    UPROPERTY(EditAnywhere, Category = "Barrier|Mesh")
    float BarrierScale = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Barrier|Collision")
    bool bEnableCollision = true;

    UPROPERTY(EditAnywhere, Category = "Barrier|Collision")
    float CollisionHeight = 200.0f;

    UPROPERTY(EditAnywhere, Category = "Barrier|Collision")
    float CollisionThickness = 50.0f;

    UPROPERTY(EditAnywhere, Category = "Barrier|Debug")
    bool bShowDebugCollision = false;

private:
    UFUNCTION()
    void BuildBarrierSpline();

    UFUNCTION()
    void SetupCollision(class USplineMeshComponent* SplineMeshComponent);

    UPROPERTY()
    TArray<class USplineMeshComponent*> SplineMeshComponents;
};
