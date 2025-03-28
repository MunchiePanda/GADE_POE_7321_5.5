#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
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

	void MoveForward(float Value);
	void MoveRight(float Value);
	void Brake(float Value);

	void Turn(float Value);
	void LookUp(float Value);


	float CurrentSpeed = 0.0f;

	UFUNCTION(BlueprintCallable, Category = "Movement") // get speed (Exposed property for speed)
	float GetSpeed() const;

	
	

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline")
	USplineComponent* Spline;
private:
	//UPROPERTY(EditAnywhere, Category = "Movement")
	//float Acceleration = 50000.0f;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float TurnSpeed = 100.0f;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* HamsterMesh;

	UPROPERTY(VisibleAnywhere, Category = "Movement")
	float AccelerationRate = 500.0f;

	UPROPERTY(VisibleAnywhere, Category = "Movement")
	float DecelerationRate = 300.0f;

	UPROPERTY(VisibleAnywhere, Category = "Movement")
	float MaxSpeed = 4000.0f;

	UPROPERTY(VisibleAnywhere, Category = "Movement")
	float BrakeForce = 800.0f;

	// Camera Components
	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* Camera;

	//reference to spline


	UPROPERTY(VisibleAnywhere, Category = "Spline")
	float MaxDistanceFromSpline = 200.f;
};