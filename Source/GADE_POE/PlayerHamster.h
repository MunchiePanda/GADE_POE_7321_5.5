#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenu_WB.h"
#include "PlayerHamster.generated.h"

class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;

class USplineComponent;
UCLASS()
class GADE_POE_API APlayerHamster : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerHamster();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void MoveForward(float Value); // Movement Controls
	void MoveRight(float Value);
	void Brake(float Value);

	void Turn(float Value); // Camera Controls
	void LookUp(float Value);

	// Pause Functionality
	void TogglePauseMenu();

	float CurrentSpeed = 0.0f; // Current speed of the hamster

	UFUNCTION(BlueprintCallable, Category = "Movement") // get speed (Exposed property for speed)
	float GetSpeed() const; 


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline")
	USplineComponent* Spline; // Reference to the spline

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> PauseMenuClass; // Reference to the pause menu widget
private:
	

	UPROPERTY(EditAnywhere, Category = "Movement")
	float TurnSpeed = 100.0f; // How quickly the hamster turns

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* HamsterMesh; // The mesh for the hamster

	UPROPERTY(VisibleAnywhere, Category = "Movement")
	float AccelerationRate = 500.0f; // How quickly the hamster accelerates

	UPROPERTY(VisibleAnywhere, Category = "Movement")
	float DecelerationRate = 300.0f; // How quickly the hamster decelerates

	UPROPERTY(VisibleAnywhere, Category = "Movement")
	float MaxSpeed = 4000.0f; // Maximum speed

	UPROPERTY(VisibleAnywhere, Category = "Movement")
	float BrakeForce = 800.0f; // How quickly the hamster stops

	// Camera Components
	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* Camera;

	//reference to spline
	UPROPERTY(VisibleAnywhere, Category = "Spline")
	float MaxDistanceFromSpline = 200.f;
	UPROPERTY(VisibleAnywhere, Category = "Spline")
	TSubclassOf<UUserWidget> PauseMenuWidgetClass;
	UPROPERTY()
	UPauseMenu_WB* PauseMenuWidget;

	bool bIsPaused = false;
};