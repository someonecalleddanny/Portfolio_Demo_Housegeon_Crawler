// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FPathSpawnAreaChoice.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FPathSpawnAreaChoice
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Spawn Choice")
	bool bSpawnUp = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Spawn Choice")
	bool bSpawnDown = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Spawn Choice")
	bool bSpawnLeft = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Spawn Choice")
	bool bSpawnRight = true;
};
