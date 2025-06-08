#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenu_WB.h"
#include "BiginnerRaceGameState.h"
#include "BeginnerRaceHUD.h"
#include "AdvancedRaceManager.h"
#include "Graph.h"
#include "WaypointManager.h"
#include "PlayerHamster.generated.h"

class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class USplineComponent;
class ABeginnerRaceGameState;
class AWaypointManager;
class AAdvancedRaceManager;

UCLASS()
class GADE_POE_API APlayerHamster : public ACharacter
{
    GENERATED_BODY()

public:
    APlayerHamster();
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Movement functions
    void MoveForward(float Value);
    void MoveRight(float Value);
    void Brake(float Value);
    void Turn(float Value);
    void LookUp(float Value);
    void SelectNextWaypoint();
    void ConfirmWaypoint();
    void TogglePauseMenu();

    // Speed control functions
    UFUNCTION(BlueprintCallable, Category = "Movement")
    float GetSpeed() const;
    
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void SetSpeed(float NewSpeed);
    
    UFUNCTION(BlueprintCallable, Category = "Movement")
    float GetMaxSpeed() const { return MaxSpeed; }

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline")
    USplineComponent* Spline;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> PauseMenuWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> EndUIWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UBeginnerRaceHUD> HUDClass;

    UPROPERTY(BlueprintReadOnly, Category = "Race")
    int32 CurrentLap;

    UPROPERTY(BlueprintReadOnly, Category = "Race")
    int32 CurrentWaypointIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Race")
    int32 PreviousWaypointIndex;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Race")
    bool bIsPlayer = true;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Race")
    int32 TotalWaypoints; // Total waypoints for graph navigation

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* HamsterMesh;

    UPROPERTY(VisibleAnywhere, Category = "Physics")
    UStaticMeshComponent* PhysicsBody;

    UPROPERTY(VisibleAnywhere, Category = "Movement")
    float CurrentSpeed;

    UPROPERTY(VisibleAnywhere, Category = "Movement")
    float MaxSpeed = 1000.0f;

    UPROPERTY(VisibleAnywhere, Category = "Movement")
    float AccelerationRate = 500.0f;

    UPROPERTY(VisibleAnywhere, Category = "Movement")
    float DecelerationRate = 300.0f;

    UPROPERTY(VisibleAnywhere, Category = "Movement")
    float BrakeForce = 800.0f;

    UPROPERTY(VisibleAnywhere, Category = "Movement")
    float TurnSpeed = 100.0f;

    UPROPERTY(VisibleAnywhere)
    USpringArmComponent* SpringArm;

    UPROPERTY(VisibleAnywhere)
    UCameraComponent* Camera;

    UPROPERTY(VisibleAnywhere, Category = "Camera")
    bool bUsePawnControlRotation = true;

    UPROPERTY(VisibleAnywhere, Category = "Spline")
    float MaxDistanceFromSpline = 200.f;

    UPROPERTY()
    UUserWidget* PauseMenuWidget;

    UPROPERTY()
    UUserWidget* EndUIWidget;

    UPROPERTY()
    UBeginnerRaceHUD* HUDWidget;

    UPROPERTY()
    bool bUseGraphNavigation;

    UPROPERTY()
    bool bIsPaused;

    UPROPERTY()
    bool bEndUIShown;

    UPROPERTY()
    AWaypointManager* WaypointManager;

    UPROPERTY()
    AAdvancedRaceManager* RaceManager;

    UPROPERTY()
    ABeginnerRaceGameState* GameState;

    UPROPERTY()
    AActor* CurrentWaypoint;

    UPROPERTY()
    TArray<AActor*> AvailableWaypoints;

    UPROPERTY()
    int32 CurrentWaypointChoice;

    UPROPERTY()
    bool bWaitingForWaypointChoice;

    float MoveDirection = 0.0f;

    void RegisterWithGameState();
    void OnWaypointReached(AActor* Waypoint);

    UFUNCTION()
    void OnPhysicsBodyOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnWaypointOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};