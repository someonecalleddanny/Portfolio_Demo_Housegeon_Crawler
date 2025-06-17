// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"

#include "Dungeon_Generation/GS_DungeonGeneration.h"
#include "Dungeon_Generation/EDungeonGenerationType.h"

#include "Components/TimelineComponent.h"

#include "EnemyAIController.generated.h"

enum ECurrent_AI_State 
{
	CheckPlayer,
	RandomPatrolMovementSelector,
	MoveForward,
	RotatedToMoveForward,
	Rotate90,
	RotateMinus90,
	Rotate180,
	Pursuit,
	Attack
};

/**
 * 
 */
UCLASS()
class HOUSEGEON_CRAWLER_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	void SetCurretXY(FIntPoint CellLocation);

protected:

	UPROPERTY()
	AGS_DungeonGeneration* myDungeonState;

	UPROPERTY()
	APawn* ControlledPawn;

	virtual void OnPossess(APawn* InPawn) override;

	UFUNCTION()
	void SpawnedEnemy();

	void Register_Enemy_Location_Cell();

private:
	FIntPoint CurrentXY;
	float NormalizedYaw = 0.f;
	float EnemyRotationSpeed = 0.4f;
	float EnemyWalkingSpeed = 1.f;

	void SetRandomRotation();

	//AI State Functions
	TMap<ECurrent_AI_State, void (AEnemyAIController::*)()> myAIStates =
	{
		{ECurrent_AI_State::RandomPatrolMovementSelector, &AEnemyAIController::Choose_Random_Patrol},
		{ECurrent_AI_State::MoveForward, &AEnemyAIController::Move_Forward}
	};


	FTimerHandle TH_NextAIEvent;
	ECurrent_AI_State MyCurrentAIState = ECurrent_AI_State::CheckPlayer;

	//If at any point the AI will stop and needs to be restarted, this function will have to be called
	void Start_AI();

	void Choose_Random_Patrol();

	void Move_Forward();

	void Notify_Delayed_Move_Forward();

	//Add the rotation you want the pawn to go by and notify the ai controller (clamped to -360 to 360 for the Yaw adder)
	void Notify_Rotate_Enemy_By_X_Amount(float YawAdder);

	void OnFinished();
	FAIManagerBatchPacket OnDelayedMoveForward();
};
