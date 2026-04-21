// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Drone.generated.h"


class UFloatingPawnMovement;
struct FInputActionValue;
class UCameraComponent;
class USpringArmComponent;
class USkeletalMeshComponent;
class UBoxComponent;

UCLASS()
class SPARTACPP07_API ADrone : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ADrone();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Component")
	UBoxComponent* BoxRootComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Component")
	USkeletalMeshComponent* BodyMeshComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Component")
	UStaticMeshComponent* WingForwardLeftMeshComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Component")
	UStaticMeshComponent* WingForwardRightMeshComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Component")
	UStaticMeshComponent* WingBackwardLeftMeshComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Component")
	UStaticMeshComponent* WingBackwardRightMeshComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Component")
	USpringArmComponent* SpringArmComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Component")
	UCameraComponent* CameraComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Component")
	UFloatingPawnMovement* FloatingPawnMovementComponent;
	
	UFUNCTION()
	void MoveAction(const FInputActionValue& value);
	UFUNCTION()
	void LookAction(const FInputActionValue& value);

public:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
