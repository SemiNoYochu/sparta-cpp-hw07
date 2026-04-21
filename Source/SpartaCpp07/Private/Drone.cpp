// Fill out your copyright notice in the Description page of Project Settings.


#include "SpartaCpp07/Public/Drone.h"

#include "EnhancedInputComponent.h"
#include "SkeletonTreeBuilder.h"
#include "SpartaPlayerController.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/SpringArmComponent.h"


// Sets default values
ADrone::ADrone()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	BoxRootComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CapsuleRootComponent"));
	SetRootComponent(BoxRootComponent);
	BoxRootComponent->SetSimulatePhysics(false);
	
	BodyMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BodyMeshComponent"));
	BodyMeshComponent->SetupAttachment(BoxRootComponent);
	BodyMeshComponent->SetSimulatePhysics(false);
	
	WingForwardLeftMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WingForwardLeftMeshComponent"));
	WingForwardLeftMeshComponent->SetupAttachment(BodyMeshComponent, TEXT("ForwardLeft"));
	WingForwardLeftMeshComponent->SetSimulatePhysics(false);
	
	WingForwardRightMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WingForwardRightComponent"));
	WingForwardRightMeshComponent->SetupAttachment(BodyMeshComponent, TEXT("ForwardRight"));
	WingForwardRightMeshComponent->SetSimulatePhysics(false);
	
	WingBackwardLeftMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WingBackwardLeftMeshComponent"));
	WingBackwardLeftMeshComponent->SetupAttachment(BodyMeshComponent, TEXT("BackwardLeft"));
	WingBackwardLeftMeshComponent->SetSimulatePhysics(false);
	
	WingBackwardRightMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WingBackwardRightMeshComponent"));
	WingBackwardRightMeshComponent->SetupAttachment(BodyMeshComponent, TEXT("BackwardRight"));
	WingBackwardRightMeshComponent->SetSimulatePhysics(false);
	
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(BoxRootComponent);
	SpringArmComponent->TargetArmLength = 300.0f;
	SpringArmComponent->bUsePawnControlRotation = true;
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;
	
	FloatingPawnMovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingPawnMovementComponent"));
	FloatingPawnMovementComponent->MaxSpeed = 8000.0f;
	FloatingPawnMovementComponent->Acceleration = 2000.0f;
	FloatingPawnMovementComponent->Deceleration = 1000.0f;
}

void ADrone::MoveAction(const FInputActionValue& value)
{
	if (!Controller) return;
	
	const FVector MoveInput = value.Get<FVector>();
	
	if (!FMath::IsNearlyZero(MoveInput.X))
	{
		AddMovementInput(GetActorForwardVector(), MoveInput.X);
	}
	
	if (!FMath::IsNearlyZero(MoveInput.Y))
	{
		AddMovementInput(GetActorRightVector(), MoveInput.Y);
	}
	
	if (!FMath::IsNearlyZero(MoveInput.Z))
	{
		AddMovementInput(FVector::UpVector, MoveInput.Z);
	}
}

void ADrone::LookAction(const FInputActionValue& value)
{
	const FVector2D LookInput = value.Get<FVector2D>();
	AddControllerYawInput(LookInput.X);
	AddControllerPitchInput(LookInput.Y);
}

// Called when the game starts or when spawned
void ADrone::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADrone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ADrone::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (ASpartaPlayerController* PlayerController = Cast<ASpartaPlayerController>(GetController()))
		{
			if (PlayerController->MoveInputAction)
			{
				EnhancedInputComponent->BindAction(
					PlayerController->MoveInputAction,
					ETriggerEvent::Triggered,
					this,
					&ADrone::MoveAction);
			}
			
			if (PlayerController->LookInputAction)
			{
				EnhancedInputComponent->BindAction(
					PlayerController->LookInputAction,
					ETriggerEvent::Triggered,
					this,
					&ADrone::LookAction);
			}
		}
	}
}

