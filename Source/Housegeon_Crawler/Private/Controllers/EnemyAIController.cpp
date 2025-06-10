// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/EnemyAIController.h"

#include "Managers/FAIManagerBatchPacket.h"

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ControlledPawn = InPawn;

	//If can't find the controlled pawn then stop the ai from doing anything as I need to control the pawn
	if (ControlledPawn)
	{
		UE_LOG(LogTemp, Display, TEXT("AI found controlled pawn"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AI NOT found controlled pawn"));
		return;
	}

	//Get my dungeon state
	myDungeonState = GetWorld()->GetGameState<AGS_DungeonGeneration>();

	//Check if valid and I was not stupid to add it to the game mode base
	if (myDungeonState)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnemyAIController possessed: %s"), *InPawn->GetName());

		//SpawnedEnemy();
		Start_AI();
		//SetRandomRotation();

		//Do the delegate call later because the possession comes after the delegate is broadcasted within the GS
		//myDungeonState->OnGridReady.AddDynamic(this, &AEnemyAIController::SpawnedEnemy);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("NOOOT HISM GAME STATE FOUND TO DISPLAY DUNGEON GENERATION!"));
	}
}

void AEnemyAIController::SpawnedEnemy()
{
	UE_LOG(LogTemp, Display, TEXT("Spawned An Enemy!"));

	if (ControlledPawn)
	{
		UE_LOG(LogTemp, Display, TEXT("The enemy has indeed been possessed!"));
		SetRandomRotation();

		Start_AI();
	}
	else 
	{
		UE_LOG(LogTemp, Error, TEXT("The enemy has NOT indeed been possessed!"));
	}
	
}

void AEnemyAIController::SetRandomRotation()
{
	//Unreal's rotation system is crap so I am brute forcing my normalised rotation for every actor spawned to face
	//0,0 of the grid which is the top left, testing I found yaw -90 faces that with my player so set it to this
	//Once I am comfortable with the navigation working, I will come back to this when I figure out how to 
	//translate relative rotations with 0 being forward on the navigation grid (This will be priority when coding
	//AI)
	FRotator CustomRotationSpawn(0.0f, 0.0f, 0.0f);


	int RandSpawn = FMath::RandRange(0, 3);

	switch (RandSpawn)
	{
	case 0:
		CustomRotationSpawn.Yaw = -90;
		//Create a normalised rotation, (useful for when checking navigation grid), 0 means forward
		NormalizedYaw = 0.f;
		break;

	case 1:
		CustomRotationSpawn.Yaw = 0.0f;
		//Create a normalised rotation, (useful for when checking navigation grid), 90 means right
		NormalizedYaw = 90.f;
		break;

	case 2:
		CustomRotationSpawn.Yaw = 90.0f;
		//Create a normalised rotation, (useful for when checking navigation grid), 0 means back
		NormalizedYaw = 180.f;
		break;

	case 3:
		CustomRotationSpawn.Yaw = 180.0f;
		//Create a normalised rotation, (useful for when checking navigation grid), 0 means back
		NormalizedYaw = 270.f;
		break;
	}

	ControlledPawn->SetActorRotation(FQuat(CustomRotationSpawn));
	//SetControlRotation(CustomRotationSpawn);
}

void AEnemyAIController::Start_AI()
{
	//Check_MC();
	Choose_Random_Patrol();
}

void AEnemyAIController::Choose_Random_Patrol()
{
	UE_LOG(LogTemp, Display, TEXT("Choosing Random Patrol"));
	int RandomInt = FMath::RandRange(0, 3);
	//Debugging
	RandomInt = 0;

	switch (RandomInt) 
	{
	case 0 :
		MyCurrentAIState = ECurrent_AI_State::MoveForward;
		Move_Forward();
		break;
	default:
		MyCurrentAIState = ECurrent_AI_State::MoveForward;
		Move_Forward();
		break;
	}
}

void AEnemyAIController::Move_Forward()
{
	FAIManagerBatchPacket BatchPacketToSend;
	BatchPacketToSend.FunctionWrapperOnFinished = [this]()
		{
			OnFinished();
		};

	myDungeonState->Notify_AI_Manager_Patrol_Batch(BatchPacketToSend);
}

void AEnemyAIController::OnFinished()
{
	UE_LOG(LogTemp, Warning, TEXT("EnemyAIController Finished event: %s"), *ControlledPawn->GetName());
	//Start_AI();
}
