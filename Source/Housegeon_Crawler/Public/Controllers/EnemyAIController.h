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

protected:

	UPROPERTY()
	AGS_DungeonGeneration* myDungeonState;

	UPROPERTY()
	APawn* ControlledPawn;

	virtual void OnPossess(APawn* InPawn) override;

	UFUNCTION()
	void SpawnedEnemy();

private:
	FIntPoint CurrentXY;
	float NormalizedYaw = 0.f;

	void SetRandomRotation();

	//AI State Functions
	TMap<ECurrent_AI_State, void (AEnemyAIController::*)()> myAIStates =
	{
		{ECurrent_AI_State::CheckPlayer, &AEnemyAIController::Check_MC},
		{ECurrent_AI_State::RandomPatrolMovementSelector, &AEnemyAIController::Choose_Random_Patrol},
		{ECurrent_AI_State::MoveForward, &AEnemyAIController::Move_Forward}
	};


	FTimerHandle TH_NextAIEvent;
	ECurrent_AI_State MyCurrentAIState = ECurrent_AI_State::CheckPlayer;

	//If at any point the AI will stop and needs to be restarted, this function will have to be called
	void Start_AI();

	/*
		Function to call the next AI State, assumes that a new AI state is made, if same that is fine, Auto corrects a 
		delay of 0.f to stop the chance of stack overflow. Also has a default delay of 0.01 seconds.
	*/
	void AI_Next_State(float Delay = 0.01f);

	//This is what calls the AI functions from whatever the current AI state is, Must be used with a timer event!
	void TimerEvent_DO_State_Function();

	void Check_MC();

	void Choose_Random_Patrol();

	void Move_Forward();
};
