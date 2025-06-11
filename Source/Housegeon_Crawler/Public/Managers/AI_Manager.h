// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Managers/FAIManagerBatchPacket.h"

#include "AI_Manager.generated.h"

UCLASS()
class HOUSEGEON_CRAWLER_API AAI_Manager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAI_Manager();

	void Set_Max_Entity_Count(int Amount);

	void Push_Patrol_Function_To_Batch(FAIManagerBatchPacket AIBatchPacket);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//A circular queue container that will batch all the pursue events that AI controllers pass through
	UPROPERTY()
	TArray<FAIManagerBatchPacket> QueuePatrolBatcher;
	int PatrolHead = 0;
	int PatrolTail = 0;

	UPROPERTY()
	TArray<FAIManagerBatchPacket> TickContainer;

	FAIManagerBatchPacket Pop_Patrol_Queue_Container();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:

	int MaxBatchableEntitiesInWorld = 0;

	int MaxAIToBatchInTick = 5;
	//Have an int that increments everytime an AI finishes their lerp, once equal to the amount to batch all have finished
	//And go to the next n amount of ai to batch
	int AllAIHaveFinished = 0;
};
