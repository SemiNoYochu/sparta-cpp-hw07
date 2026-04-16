// Fill out your copyright notice in the Description page of Project Settings.


#include "SpartaCpp07/Public/SpartaGameMode.h"

#include "SpartaCpp07/Public/Drone.h"

ASpartaGameMode::ASpartaGameMode()
{
	DefaultPawnClass = ADrone::StaticClass();
}
