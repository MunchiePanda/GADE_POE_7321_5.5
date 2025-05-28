#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenu_WB.h"
#include "BiginnerRaceGameState.h"
#include "BeginnerRaceHUD.h"
#include "SFXManager.h" // Include SFXManager
#include "PlayerHamster.generated.h"

// Forward declarations
class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class USplineComponent;
class AWaypointManager;
class ABeginnerRaceGameState;
class SFXManager;

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

    // Player movement functions 
    void MoveForward(float Value);
    void MoveRight(float Value);
    void Brake(float Value);

    // Camera control functions
    void Turn(float Value);
    void LookUp(float Value);

    void TogglePauseMenu();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    float GetSpeed() const;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline")
    USplineComponent* Spline; // Spline component for movement

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> PauseMenuWidgetClass; // Pause menu class

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> EndUIWidgetClass; // End UI class

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UBeginnerRaceHUD> HUDClass; // HUD class

    UPROPERTY(BlueprintReadOnly, Category = "Race")
    int32 CurrentLap; // Current lap number 

    UPROPERTY(BlueprintReadOnly, Category = "Race")
    int32 CurrentWaypointIndex; // Current waypoint index 

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Race")
    bool bIsPlayer = true; // Flag to identify player

private:
    // Components 
    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* HamsterMesh; // Hamster mesh component

    UPROPERTY(VisibleAnywhere, Category = "Movement")
    float TurnSpeed = 100.0f; // Turn speed

    UPROPERTY(VisibleAnywhere, Category = "Movement")
    float AccelerationRate = 500.0f; // Acceleration rate

    UPROPERTY(VisibleAnywhere, Category = "Movement")
    float DecelerationRate = 300.0f; // Deceleration rate

    UPROPERTY(VisibleAnywhere, Category = "Movement")
    float MaxSpeed = 4000.0f; // Maximum speed

    UPROPERTY(VisibleAnywhere, Category = "Movement")
    float BrakeForce = 800.0f; // Brake force

    // Camera and spring arm 
    UPROPERTY(VisibleAnywhere)
    USpringArmComponent* SpringArm;

    UPROPERTY(VisibleAnywhere)
    UCameraComponent* Camera;

    UPROPERTY(VisibleAnywhere, Category = "Spline")
    float MaxDistanceFromSpline = 200.f; // Maximum distance from the spline

    UPROPERTY()
    UUserWidget* PauseMenuWidget; // Pause menu widget

    UPROPERTY()
    UUserWidget* EndUIWidget; // End UI widget

    UPROPERTY()
    UBeginnerRaceHUD* HUDWidget; // HUD widget

    bool bIsPaused = false; // Flag to check if the game is paused

    bool bEndUIShown = false; // Flag to check if End UI is shown

    float CurrentSpeed = 0.0f; // Current speed

    UPROPERTY()
    AWaypointManager* WaypointManager; // Waypoint manager

    UPROPERTY()
    ABeginnerRaceGameState* GameState; // Game state reference

    void RegisterWithGameState(); // Register with game state 
    void OnWaypointReached(AActor* Waypoint); // Called when a waypoint is reached

    UPROPERTY(VisibleAnywhere, Category = "Audio")
    USFXManager* SFXManager; // Add SFXManager as a component
};