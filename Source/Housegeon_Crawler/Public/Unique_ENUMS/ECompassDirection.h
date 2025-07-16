// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


UENUM(BlueprintType) // Exposes the enum to Blueprints
enum class ECompassDirection : uint8
{
	North UMETA(DisplayName = "North"),
	NorthEast UMETA(DisplayName = "NorthEast"),
	East UMETA(DisplayName = "East"),
	SouthEast UMETA(DisplayName = "SouthEast"),
	South UMETA(DisplayName = "South"),
	SouthWest UMETA(DisplayName = "SouthWest"),
	West UMETA(DisplayName = "West"),
	NorthWest UMETA(DisplayName = "NorthWest")
};
