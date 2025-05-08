// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
UENUM(BlueprintType) // Exposes the enum to Blueprints
enum class EPathTraversalType : uint8
{
	Random UMETA(DisplayName = "Random"),
	LineBased UMETA(DisplayName = "LineBased"),
	StairBased UMETA(DisplayName = "StairBased")
};
