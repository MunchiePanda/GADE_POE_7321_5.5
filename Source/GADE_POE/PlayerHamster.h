#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenu_WB.h"
#include "BiginnerRaceGameState.h"
#include "PlayerHamster.generated.h"

class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class USplineComponent;
class AWaypointManager;
class ABiginnerRaceGameState;
class UBeginnerRaceHUD;

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
    TSubclassOf<UUserWidget> PauseMenuClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UBeginnerRaceHUD> HUDClass;

    UPROPERTY(BlueprintReadOnly, Category = "Race")
    int32 CurrentLap;

    UPROPERTY(BlueprintReadOnly, Category = "Race")
    int32 CurrentWaypointIndex;

private:
    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* HamsterMesh;

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

    UPROPERTY(VisibleAnywhere, Category = "Spline")
    TSubclassOf<UUserWidget> PauseMenuWidgetClass;

    UPROPERTY()
    UPauseMenu_WB* PauseMenuWidget;

    UPROPERTY()
    UUserWidget* HUDWidget;

    bool bIsPaused = false;

    float CurrentSpeed = 0.0f;

    UPROPERTY()
    AWaypointManager* WaypointManager;

    UPROPERTY()
    ABeginnerRaceGameState* GameState;

    void RegisterWithGameState();
    void OnWaypointReached(AActor* Waypoint);
};