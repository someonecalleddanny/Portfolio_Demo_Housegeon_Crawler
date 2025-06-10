// Fill out your copyright notice in the Description page of Project Settings.


#include "Managers/AI_Manager.h"

// Sets default values
AAI_Manager::AAI_Manager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void AAI_Manager::Set_Max_Entity_Count(int Amount)
{
	//Set the max amount of entities that the queue batchers will hold, The idea is that the queue systems are static in
	//size when the dungeon is generated, for now I'll only have a set amount of enemies that are spawned at the start
	//No new ones will be spawned but I'll have to increase the max entity amount at start of generation
	MaxBatchableEntitiesInWorld = Amount;

	QueuePatrolBatcher.SetNum(MaxBatchableEntitiesInWorld);
}

void AAI_Manager::Add_Patrol_Function_To_Batch(FAIManagerBatchPacket AIBatchPacket)
{
	//Check if the inputted AI controller is valid
	if (!(AIBatchPacket.ControlledPawnRef.IsValid()))
	{
		UE_LOG(LogTemp, Error, TEXT("Inputted AI controller not valid!!!"));
		return;
	}

	//First check if the pursue event batcher is full (It should not be but at least I can debug where the wrong things
	//are going) This is because each AI controller should only batch one event which the max entity counts for
	if (PatrolHead == PatrolTail && QueuePatrolBatcher[PatrolHead].ControlledPawnRef.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Pursue Batcher container in ai manager is full, should not happen!"));
		return;
	}

	QueuePatrolBatcher[PatrolTail] = AIBatchPacket;

	//If reached the end of the array with the tail, wrap around to the start of the array as am using a circular queue
	//Useful wrapper, use a modulus to check if the next index moved to is out of bounds, if index 10 = size 10 set to 0
	PatrolTail = (PatrolTail + 1) % QueuePatrolBatcher.Num();

	//So, if the queue was empty, set it to false to allow the tick to do its business
	if (bQueuePatrolEmpty) 
	{
		bQueuePatrolEmpty = false;
	}
}

// Called when the game starts or when spawned
void AAI_Manager::BeginPlay()
{
	Super::BeginPlay();
	
	//GetWorld()->GetTimerManager().SetTimer(TH_PursueBatcher, this, &AAI_Manager::Timer_Batch_Pursue_Events, 1.f, true);

}

// Called every frame
void AAI_Manager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Instantly skip if the queue is empty, set to empty at the end of the tick if you want to check how it is set to true
	if (bQueuePatrolEmpty) return;

	//Create a temp head int as I will be going and lerping through every nth controlled pawn 
	int TempHead = PatrolHead;

	for (int i = 0; i < MaxAIToBatchInTick; i++)
	{
		//Make sure that the head does not go out of bounds by making it wrap round to the start
		TempHead = (PatrolHead + i) % QueuePatrolBatcher.Num();

		//Set a temp packet that is addressed to the indexed ai packet for readability
		FAIManagerBatchPacket& TempPacket = QueuePatrolBatcher[TempHead];

		//Check if further ai
		//have been batched via the ai controller, if not instantly go to the next item (possibility of one ai finishing
		//their movement at a different time), In the future I will add a skipper so that really slow ai won't slog everyone
		//down (because I wait until all n ai items have finished until batching the next n amount)
		if (!(TempPacket.ControlledPawnRef.IsValid()))
		{
			continue;
		}

		//Looks ugly but I need to compare when all three floats are relatively the same (floats can be fickle when checking
		// if exactly equal)
		if(FMath::IsNearlyEqual(TempPacket.StartX, TempPacket.EndX)
			&& FMath::IsNearlyEqual(TempPacket.StartY, TempPacket.EndY)
				&& FMath::IsNearlyEqual(TempPacket.StartZ, TempPacket.EndZ))
		{
			//Check if the function wrapper has been set
			if (TempPacket.FunctionWrapperOnFinished)
			{
				//Call the function wrapper, there is a chance that if I am stupid and forget to wrap the whole game crashes
				//but I will minimise my stupidity
				TempPacket.FunctionWrapperOnFinished();
				//After calling the event I want to pop out the controlled pawn
				TempPacket.ControlledPawnRef = nullptr;
				//Also for extra safety pop out the function wrapper so not called by mistake
				TempPacket.FunctionWrapperOnFinished = nullptr;
				AllAIHaveFinished++;
			}
		}
	}

	//After checking all n AI, check if all of them have finished their task, then go to the next batch
	if (AllAIHaveFinished == MaxAIToBatchInTick) 
	{
		//Set back to 0 to then check the next n amount of ai to batch events to
		AllAIHaveFinished = 0;

		//Then add to the next index of what would be the max (wrap if needed)
		PatrolHead = (PatrolHead + MaxAIToBatchInTick) % QueuePatrolBatcher.Num();

		//Then check if the new head index has an empty controlled pawn, if yes then that means the queue is currently empty
		//and waiting for the next controlled pawn to be added
		if (!(QueuePatrolBatcher[PatrolHead].ControlledPawnRef.IsValid())) 
		{
			bQueuePatrolEmpty = true;
		}
	}
}

