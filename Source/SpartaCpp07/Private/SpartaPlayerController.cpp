// Fill out your copyright notice in the Description page of Project Settings.


#include "SpartaCpp07/Public/SpartaPlayerController.h"

#include "EnhancedInputSubsystems.h"

ASpartaPlayerController::ASpartaPlayerController()
	: InputMappingContext(nullptr)
	, MoveInputAction(nullptr)
	, LookInputAction(nullptr)
{
}

void ASpartaPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (InputMappingContext)
			{
				Subsystem->AddMappingContext(InputMappingContext, 0);
			}
		}
	}
}
