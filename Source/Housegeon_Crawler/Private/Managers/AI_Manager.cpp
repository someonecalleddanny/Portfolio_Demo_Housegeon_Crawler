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

void AAI_Manager::Push_Patrol_Function_To_Batch(FAIManagerBatchPacket AIBatchPacket)
{
	if (QueuePatrolBatcher.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("You forgot to set the patrol queue batcher size dumba**"));
		return;
	}
	//Check if the inputted AI controller is valid
	if (!(AIBatchPacket.ControlledPawnRef.IsValid()))
	{
		UE_LOG(LogTemp, Error, TEXT("AI Manager: Inputted AI controller not valid!!!"));
		return;
	}

	//First check if the pursue event batcher is full (It should not be but at least I can debug where the wrong things
	//are going) This is because each AI controller should only batch one event which the max entity counts for
	if (PatrolHead == PatrolTail && QueuePatrolBatcher[PatrolHead].ControlledPawnRef.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Pursue Batcher container in ai manager is full, should not happen!"));
		return;
	}

	//UE_LOG(LogTemp, Display, TEXT("Pushed Patrol ai packet to ai manager"));

	QueuePatrolBatcher[PatrolTail] = AIBatchPacket;

	//If reached the end of the array with the tail, wrap around to the start of the array as am using a circular queue
	//Useful wrapper, use a modulus to check if the next index moved to is out of bounds, if index 10 = size 10 set to 0
	PatrolTail = (PatrolTail + 1) % QueuePatrolBatcher.Num();
}

// Called when the game starts or when spawned
void AAI_Manager::BeginPlay()
{
	Super::BeginPlay();
	
	//GetWorld()->GetTimerManager().SetTimer(TH_PursueBatcher, this, &AAI_Manager::Timer_Batch_Pursue_Events, 1.f, true);
	TickContainer.SetNum(MaxAIToBatchInTick);
}

FAIManagerBatchPacket AAI_Manager::Pop_Patrol_Queue_Container()
{
	FAIManagerBatchPacket ReturnedPacket;

	//Check if the pawn controlled is valid, a major checker as a lerp would not function without it
	if (QueuePatrolBatcher[PatrolHead].ControlledPawnRef.IsValid()) 
	{
		//UE_LOG(LogTemp, Display, TEXT("Pop valid for ai batch packet"));

		//Set the returned packet to what is at the current head and then increment the head (allowing for wrapping)
		ReturnedPacket = QueuePatrolBatcher[PatrolHead];

		FAIManagerBatchPacket UnInitPacket;
		QueuePatrolBatcher[PatrolHead] =  UnInitPacket;

		PatrolHead = (PatrolHead + 1) % QueuePatrolBatcher.Num();
		return ReturnedPacket;
	}

	//if there is nothing at the head to pop, means that the queue is empty
	//UE_LOG(LogTemp, Display, TEXT("The patrol queue is empty"));
	return ReturnedPacket;
}

// Called every frame
void AAI_Manager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	for (int i = 0; i < TickContainer.Num(); i++)
	{
		//Set a temp packet that is addressed to the indexed ai packet for readability
		FAIManagerBatchPacket& TempPacket = TickContainer[i];

		//The tick constantly checks if new items have been added to the patrol queue, if yes, add to the tick container
		// by popping the head from the patrol queue. Don't worry, if nothing at head, will return an empty AI packet which
		// will have an invalid controlledpawnref which is what the if function constantly checks. The controlled pawn having
		// a pointer essentially means that the ai manager has set something up, there are checks below to stop any stupid
		// coding and crashing (Such as not creating a function wrapper correctly)
		//And, for safety, execute the function on the next tick cycle.
		if (!(TempPacket.ControlledPawnRef.IsValid()))
		{
			TickContainer[i] = Pop_Patrol_Queue_Container();
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
				//Call the function wrapper to the ai controller, if not valid, will crash the game but that's why I validate
				//the controlled pawn as, if that dies, so will the ai controller
				TempPacket.FunctionWrapperOnFinished();
				
				
				//After finishing, the AI has a new state, so I am assuming it has already been pushed to the AI patrol
				//queue by the time I get to the function below, this means I can pop whatever is at the head and execute it
				//on the next tick cycle, the pop function does check if the queue is empty so will not unecessarily
				//increment the head index
				TickContainer[i] = Pop_Patrol_Queue_Container();
			}
		}
	}
}

