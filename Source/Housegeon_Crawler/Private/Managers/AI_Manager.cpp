// Fill out your copyright notice in the Description page of Project Settings.


#include "Managers/AI_Manager.h"

// Sets default values
AAI_Manager::AAI_Manager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

void AAI_Manager::Set_Max_Entity_Count(int Amount)
{
	//Set the max amount of entities that the queue batchers will hold, The idea is that the queue systems are static in
	//size when the dungeon is generated, for now I'll only have a set amount of enemies that are spawned at the start
	//No new ones will be spawned but I'll have to increase the max entity amount at start of generation
	MaxBatchableEntitiesInWorld = Amount;

	PursueEventBatchQueueContainer.SetNum(MaxBatchableEntitiesInWorld);
}

void AAI_Manager::Add_Pursue_Function_To_Batch(AController* AIControllerToBatch)
{
	//Check if the inputted AI controller is valid
	if (!(AIControllerToBatch)) 
	{
		UE_LOG(LogTemp, Error, TEXT("Inputted AI controller not valid!!!"));
		return;
	}

	//First check if the pursue event batcher is full (It should not be but at least I can debug where the wrong things
	//are going) This is because each AI controller should only batch one event which the max entity counts for
	if (PursueHead == PursueTail && PursueEventBatchQueueContainer[PursueHead]) 
	{
		UE_LOG(LogTemp, Error, TEXT("Pursue Batcher container in ai manager is full, should not happen!"));
		return;
	}

	PursueEventBatchQueueContainer[PursueTail] = AIControllerToBatch;

	//If reached the end of the array with the tail, wrap around to the start of the array as am using a circular queue
	//Useful wrapper, use a modulus to check if the next index moved to is out of bounds, if index 10 = size 10 set to 0
	PursueTail = (PursueTail + 1) % PursueEventBatchQueueContainer.Num();
}

// Called when the game starts or when spawned
void AAI_Manager::BeginPlay()
{
	Super::BeginPlay();
	
	GetWorld()->GetTimerManager().SetTimer(TH_PursueBatcher, this, &AAI_Manager::Timer_Batch_Pursue_Events, 1.f, true);
}

void AAI_Manager::Timer_Batch_Pursue_Events()
{
	UE_LOG(LogTemp, Display, TEXT("Batching pursue events"));
}

// Called every frame
void AAI_Manager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

