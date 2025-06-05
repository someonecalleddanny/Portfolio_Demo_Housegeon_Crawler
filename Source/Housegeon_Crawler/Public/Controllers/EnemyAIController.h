// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"

#include "Dungeon_Generation/GS_DungeonGeneration.h"
#include "Dungeon_Generation/EDungeonGenerationType.h"

#include "Components/TimelineComponent.h"

#include "EnemyAIController.generated.h"

/**
 * 
 */
UCLASS()
class HOUSEGEON_CRAWLER_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

protected:

	UPROPERTY()
	AGS_DungeonGeneration* myDungeonState;

	virtual void OnPossess(APawn* InPawn) override;

	UFUNCTION()
	void SpawnedEnemy();

private:
	FIntPoint CurrentXY;
	float NormalizedYaw = 0.f;

	void SetRandomRotation();
};
