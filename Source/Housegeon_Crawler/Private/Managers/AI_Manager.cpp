// Fill out your copyright notice in the Description page of Project Settings.


#include "Managers/AI_Manager.h"

// Sets default values
AAI_Manager::AAI_Manager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Jesus Christ Almighty, simply throttling the actor tick is a game changer for performance. This single line of code
	//gained me 10 fps which is absolutely crazy to think about. 110fps to 120 consistent. Mad
	PrimaryActorTick.TickInterval = 1.f / 45.f;
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
	if (!(AIBatchPacket.Get_Pawn().IsValid()))
	{
		UE_LOG(LogTemp, Error, TEXT("AI Manager: Inputted AI controller not valid!!!"));
		return;
	}

	//First check if the pursue event batcher is full (It should not be but at least I can debug where the wrong things
	//are going) This is because each AI controller should only batch one event which the max entity counts for
	if (PatrolHead == PatrolTail && QueuePatrolBatcher[PatrolHead].Get_Pawn().IsValid())
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

	//So, if the current head is empty, don't increment or anything, just return a null ai packet
	if (!QueuePatrolBatcher[PatrolHead].Get_Pawn().IsValid()) 
	{
		UE_LOG(LogTemp, Warning, TEXT("The patrol queue batcher in the ai manager is empty, awaiting new commands!"));
		return ReturnedPacket;
	}

	//UE_LOG(LogTemp, Display, TEXT("Pop valid for ai batch packet"));

	//Set the returned packet to what is at the current head of the queue
	ReturnedPacket = QueuePatrolBatcher[PatrolHead];

	//Then replace the current head index with a null batch struct to mark it as "undirty"
	FAIManagerBatchPacket UnInitPacket;
	QueuePatrolBatcher[PatrolHead] = UnInitPacket;

	//Then increment the Patrol Head for the next pop (allowing for wrapping around the queue container)
	PatrolHead = (PatrolHead + 1) % QueuePatrolBatcher.Num();

	//A bit more technical here, some events need to be called as they are popped as they need frame perfect execution,
	//This is where the batch packet gets overwritten by what actually needs to be executed
	if (ReturnedPacket.Is_A_Delayed_Batch_Packet()) 
	{
		ReturnedPacket = ReturnedPacket.Call_OnDelayedFunction();
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
		if (!(TempPacket.Get_Pawn().IsValid()))
		{
			TickContainer[i] = Pop_Patrol_Queue_Container();
			continue;
		}

		//Get my three floats which will be translated into FVectors or FRotators
		FThreeFloatContainer StartFloats = TempPacket.Get_Start_XYZ();
		FThreeFloatContainer EndFloats = TempPacket.Get_End_XYZ();

		// Calculate new alpha with deltatime to ensure that frame drops don't affect speed
		float AlphaIncrement = DeltaTime / TempPacket.Get_Time_To_Finish();
		float NewAlpha = TempPacket.Get_Alpha() + AlphaIncrement;

		//Save the current alpha for the next tick cycle
		TempPacket.Set_Alpha(NewAlpha);

		//Create a smooth step ease for the alpha to not be robotic (Sorry, looked up equation for smooth step to make
		//this work)
		float EasedAlpha = NewAlpha * NewAlpha * (3 - 2 * NewAlpha);

		//Then either set the rotation or location of the controlled pawn
		if (TempPacket.Is_Rotating()) 
		{
			//Put in the z for where the y should kinda be because Unreal puts yaw in the second slot when in bp it is on
			//the third, (ONLY rotating yaw!)
			FRotator StartRotation = FRotator(0.f, StartFloats.Z, 0.f);
			FRotator EndRotation = FRotator(0.f, EndFloats.Z, 0.f);

			FRotator NewRotation = FMath::Lerp(StartRotation, EndRotation, EasedAlpha);

			TempPacket.Get_Pawn()->SetActorRotation(NewRotation);
		}
		else 
		{
			//Here, create the FVectors for the lerp
			FVector StartLocation = FVector(StartFloats.X, StartFloats.Y, StartFloats.Z);
			FVector EndLocation = FVector(EndFloats.X, EndFloats.Y, EndFloats.Z);

			// Lerp location
			FVector NewLocation = FMath::Lerp(StartLocation, EndLocation, EasedAlpha);

			TempPacket.Get_Pawn()->SetActorLocation(NewLocation);
		}

		//Check if the alpha is at 1, if yes, that means the lerp has finished which means that the temp packet can finally
		//call the onfinished event
		if(TempPacket.Get_Alpha() >= 1.f)
		{
			//Call the function wrapper to the ai controller, if not valid, will crash the game but that's why I validate
			//the controlled pawn as, if that dies, so will the ai controller (ALSO, checks in the struct if the wrapper is
			//valid)
			TempPacket.Call_OnFinished();

			//After finishing, the AI has a new state, so I am assuming it has already been pushed to the AI patrol
			//queue by the time I get to the function below, this means I can pop whatever is at the head and execute it
			//on the next tick cycle, the pop function does check if the queue is empty so will not unecessarily
			//increment the head index
			TickContainer[i] = Pop_Patrol_Queue_Container();
		}
	}
}

