// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */

UENUM(BlueprintType) // Exposes the enum to Blueprints
enum class EDungeonGenerationType : uint8
{
	Wall UMETA(DisplayName = "Wall"),
	Floor UMETA(DisplayName = "Floor"),
	Spawn UMETA(DisplayName = "Spawn"),
	EndPoint UMETA(DisplayName = "EndPoint"),
	ChestPOI UMETA(DisplayName = "ChestPOI")
};
