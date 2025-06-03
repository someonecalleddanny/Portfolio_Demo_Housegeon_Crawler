// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Entities/Enemy.h"

#include "FEnemySpawnData.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FEnemySpawnData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++ Properties")
	TArray<TSubclassOf<AEnemy>> AllEnemies;
};