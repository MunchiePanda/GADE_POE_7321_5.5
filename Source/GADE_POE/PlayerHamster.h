#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenu_WB.h"
#include "BiginnerRaceGameState.h"
#include "BeginnerRaceHUD.h"
#include "SFXManager.h"
#include "AdvancedRaceManager.h"
#include "Graph.h"
#include "WaypointManager.h"
#include "PlayerHamster.generated.h"

class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class USplineComponent;
class ABeginnerRaceGameState;
class USFXManager;
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

    void MoveForward(float Value);
    void MoveRight(float Value);
    void Brake(float Value);

    void Turn(float Value);
    void LookUp(float Value);

    void TogglePauseMenu();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    float GetSpeed() const;

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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Race")
    bool bIsPlayer = true;

    float CurrentSpeed = 0.0f;
private:
    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* HamsterMesh; // Visual mesh

    UPROPERTY(VisibleAnywhere, Category = "Physics")
    UStaticMeshComponent* PhysicsBody; // Physics-enabled body for collisions

    UPROPERTY(VisibleAnywhere, Category = "Movement")
    float TurnSpeed = 100.0f;

    UPROPERTY(VisibleAnywhere, Category = "Movement")
    float AccelerationRate = 500.0f;

    UPROPERTY(VisibleAnywhere, Category = "Movement")
    float DecelerationRate = 300.0f;

    UPROPERTY(VisibleAnywhere, Category = "Movement")
    float MaxSpeed = 4000.0f;

    UPROPERTY(VisibleAnywhere, Category = "Movement")
    float BrakeForce = 800.0f;

    UPROPERTY(VisibleAnywhere)
    USpringArmComponent* SpringArm;

    UPROPERTY(VisibleAnywhere)
    UCameraComponent* Camera;

    UPROPERTY(VisibleAnywhere, Category = "Spline")
    float MaxDistanceFromSpline = 200.f;

    UPROPERTY()
    UUserWidget* PauseMenuWidget;

    UPROPERTY()
    UUserWidget* EndUIWidget;

    UPROPERTY()
    UBeginnerRaceHUD* HUDWidget;

    bool bIsPaused = false;
    bool bEndUIShown = false;
    

    UPROPERTY()
    AWaypointManager* WaypointManager; // Keep for compatibility with other levels

    UPROPERTY()
    AAdvancedRaceManager* RaceManager; // Add for advanced map

    UPROPERTY()
    ABeginnerRaceGameState* GameState;

    UPROPERTY(VisibleAnywhere, Category = "Audio", Meta = (AllowPrivateAccess = "true"))
    USFXManager* SFXManager;

    bool bUseGraphNavigation = false; // Flag to determine navigation mode

    UPROPERTY()
    AActor* CurrentWaypoint; // Used for graph navigation

    FVector MoveDirection = FVector::ZeroVector;

    void RegisterWithGameState();
    void OnWaypointReached(AActor* Waypoint);
};