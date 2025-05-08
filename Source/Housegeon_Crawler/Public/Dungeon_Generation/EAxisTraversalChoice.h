// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
UENUM(BlueprintType) // Exposes the enum to Blueprints
enum class EAxisTraversalChoice : uint8
{
	Random UMETA(DisplayName = "Random"),
	RowFirst UMETA(DisplayName = "RowFirst"),
	ColumnFirst UMETA(DisplayName = "ColumnFirst")
};