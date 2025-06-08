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

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Called to bind functionality to input actions
    void MoveForward(float Value);
    void MoveRight(float Value);
    void Brake(float Value);

    void Turn(float Value);
    void LookUp(float Value);
    void SelectNextWaypoint(); // New function to cycle through waypoint choices
    void ConfirmWaypoint(); // New function to confirm waypoint selection

    void TogglePauseMenu();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    float GetSpeed() const; // Function to get the speed

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline")
    USplineComponent* Spline; // Spline component

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> PauseMenuWidgetClass; // Class for the pause menu widget

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> EndUIWidgetClass; // Class for the end UI widget

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UBeginnerRaceHUD> HUDClass; // Class for the HUD widget

    UPROPERTY(BlueprintReadOnly, Category = "Race")
    int32 CurrentLap; // Current lap

    UPROPERTY(BlueprintReadOnly, Category = "Race")
    int32 CurrentWaypointIndex; // Current waypoint index

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Race")
    bool bIsPlayer = true; // Flag to indicate if this is the player hamster

    float CurrentSpeed = 0.0f; // Current speed

private:
    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* HamsterMesh; // Visual mesh

    UPROPERTY(VisibleAnywhere, Category = "Physics")
    UStaticMeshComponent* PhysicsBody; // Physics-enabled body for collisions

    UPROPERTY(VisibleAnywhere, Category = "Movement")
    float TurnSpeed = 100.0f; // Speed at which the hamster turns

    UPROPERTY(VisibleAnywhere, Category = "Movement")
    float AccelerationRate = 500.0f; // Rate at which the hamster accelerates when moving

    UPROPERTY(VisibleAnywhere, Category = "Movement")
    float DecelerationRate = 300.0f; // Rate at which the hamster slows down when not accelerating

    UPROPERTY(VisibleAnywhere, Category = "Movement")
    float MaxSpeed = 4000.0f; // Maximum speed

    UPROPERTY(VisibleAnywhere, Category = "Movement")
    float BrakeForce = 800.0f; // Force applied when braking

    UPROPERTY(VisibleAnywhere)
    USpringArmComponent* SpringArm; // Spring arm for camera positioning

    UPROPERTY(VisibleAnywhere)
    UCameraComponent* Camera; // Camera component for the player

    UPROPERTY(VisibleAnywhere, Category = "Camera")
    bool bUsePawnControlRotation = true; // Allow pawn to control rotation

    UPROPERTY(VisibleAnywhere, Category = "Spline")
    float MaxDistanceFromSpline = 200.f; // Maximum distance from the spline for navigation

    UPROPERTY()
    UUserWidget* PauseMenuWidget; // Pause menu

    UPROPERTY()
    UUserWidget* EndUIWidget; // End UI widget

    UPROPERTY()
    UBeginnerRaceHUD* HUDWidget; // HUD widget

    UPROPERTY()
    bool bUseGraphNavigation;

    UPROPERTY()
    bool bIsPaused;

    UPROPERTY()
    bool bEndUIShown;

    UPROPERTY()
    class AWaypointManager* WaypointManager;

    UPROPERTY()
    class AAdvancedRaceManager* RaceManager;

    UPROPERTY()
    ABeginnerRaceGameState* GameState;

    UPROPERTY()
    AActor* CurrentWaypoint; // Used for graph navigation

    // New properties for waypoint choice system
    UPROPERTY()
    TArray<AActor*> AvailableWaypoints; // List of available waypoint choices

    UPROPERTY()
    int32 CurrentWaypointChoice; // Index of currently highlighted waypoint choice

    UPROPERTY()
    bool bWaitingForWaypointChoice; // Whether we're waiting for the player to choose a waypoint

    FVector MoveDirection = FVector::ZeroVector;

    void RegisterWithGameState();
    void OnWaypointReached(AActor* Waypoint);

    // Collision event for physics body overlap
    UFUNCTION()
    void OnPhysicsBodyOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnWaypointOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};