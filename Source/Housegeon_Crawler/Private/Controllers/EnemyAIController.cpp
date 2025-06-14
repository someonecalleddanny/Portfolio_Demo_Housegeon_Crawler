// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/EnemyAIController.h"

#include "Managers/FAIManagerBatchPacket.h"

void AEnemyAIController::SetCurretXY(FIntPoint CellLocation)
{
	CurrentXY = CellLocation;
}

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

		//Do the delegate call later because the possession comes after the delegate is broadcasted within the GS
		myDungeonState->OnAIManagerReady.AddDynamic(this, &AEnemyAIController::SpawnedEnemy);
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

		Register_Enemy_Location_Cell();

		Start_AI();
	}
	else 
	{
		UE_LOG(LogTemp, Error, TEXT("The enemy has NOT indeed been possessed!"));
	}
	
}

void AEnemyAIController::Register_Enemy_Location_Cell()
{
	if (!ControlledPawn) return;

	FVector RawLocation = ControlledPawn->GetActorLocation();

	CurrentXY.X = RawLocation.X / 400.f;
	CurrentXY.Y = RawLocation.Y / 400.f;

	/*
		Will need to transfer to the GS
	*/
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

	//Debug to check if spawn is working
	//UE_LOG(LogTemp, Warning, TEXT("EnemyAIController: %s"), *ControlledPawn->GetName());
	//UE_LOG(LogTemp, Warning, TEXT("Current Normalised Yaw = %f"), NormalizedYaw);
}

void AEnemyAIController::Start_AI()
{
	//Check_MC();
	Choose_Random_Patrol();
}

void AEnemyAIController::Choose_Random_Patrol()
{
	//UE_LOG(LogTemp, Display, TEXT("Choosing Random Patrol"));
	int RandomInt = FMath::RandRange(0, 3);
	//Debugging
	//RandomInt = 3;

	switch (RandomInt) 
	{
	case 0 :
		MyCurrentAIState = ECurrent_AI_State::MoveForward;
		Move_Forward();
		break;

	case 1:
		MyCurrentAIState = ECurrent_AI_State::Rotate90;
		Notify_Rotate_Enemy_By_X_Amount(90.f);
		break;

	case 2:
		MyCurrentAIState = ECurrent_AI_State::RotateMinus90;
		Notify_Rotate_Enemy_By_X_Amount(-90.f);
		break;

	case 3:
		MyCurrentAIState = ECurrent_AI_State::Rotate180;
		Notify_Rotate_Enemy_By_X_Amount(180.f);
		break;

	default:
		UE_LOG(LogTemp, Error, TEXT("Your random patrol index is invalid within your ai controller"));
		break;
	}
}

void AEnemyAIController::Move_Forward()
{
	//UE_LOG(LogTemp, Display, TEXT("Moving forward"));

	FAIManagerBatchPacket BatchPacketToSend;

	FVector StartLocation = ControlledPawn->GetActorLocation();
	FVector EndLocation;

	if (myDungeonState->Can_Move_Forward(CurrentXY.X, CurrentXY.Y, NormalizedYaw)) 
	{
		myDungeonState->Moving_Forward(CurrentXY.X, CurrentXY.Y, NormalizedYaw);
	}
	else 
	{
		//Right, so if I cannot find a place to move forward to, keep choosing a random adder yaw until finding which yaw
		//The enemy can move forward on, since this will require an animation, it will have to be batched but on finished
		//I will make sure to call move forward function again so that the enemy can move forward on next batch
		TArray<float> PossibleAdderYaws = { 90.f, 180.f, -90.f };
		int RandIndex;

		//Loop 3 times because the game only supports 4 yaw rotations (your current yaw could not move so need to check the
		//other 3 if they are movable, there is a chance that the AI can be boxed in from walls and other AI blocking
		//paths but I will deal with that below)
		for (int i = 0; i < 3; i++) 
		{
			RandIndex = FMath::RandRange(0, PossibleAdderYaws.Num() - 1);
			float RandomAdderYaw = PossibleAdderYaws[RandIndex];
			PossibleAdderYaws.RemoveAt(RandIndex);

			float PotentialYaw = RandomAdderYaw + NormalizedYaw;
			float ErrorTolerance = 0.5f;

			//Wrap the potential yaw to only have a value of 0 - 360 (- error tolerance) Could use FMod but I really want
			//to aggressively make this as optimised as possible, I like saving the small overhead of not using the FMath
			//class
			if (PotentialYaw >= 360.f - ErrorTolerance) 
			{
				PotentialYaw -= 360.f;
			}
			else if (PotentialYaw <= 0.f - ErrorTolerance)
			{
				PotentialYaw += 360.f;
			}

			//If I found a suitable location in which I can move forward to, call the rotate adder event which
			//batches the rotation for the ai manager to rotate the enemy
			if(myDungeonState->Can_Move_Forward(CurrentXY.X, CurrentXY.Y, PotentialYaw))
			{
				//Change the current ai state to rotatedtomoveforward, because on finish it will call this move forward
				//function again to check if the enemy can move forward
				MyCurrentAIState = ECurrent_AI_State::RotatedToMoveForward;
				Notify_Rotate_Enemy_By_X_Amount(RandomAdderYaw);
				return;
			}
		}
		//if the for loop could not find any yaw locations that the enemy could move from, means that it is stuck and
		//just needs to wait until the way is clear
		UE_LOG(LogTemp, Warning, TEXT("Enemy Is boxed in, going to just stay still for this tick cycle!"));
	}

	//Bind your onfinished function
	TFunction<void()> TempFunctionWrapper = [this]()
		{
			OnFinished();
		};

	//Create the end location for the lerp (Don't really need to set z but better to be safer than sorry if Unreal does
	//Unreal things to uninitted variables
	EndLocation.X = CurrentXY.X * 400.f;
	EndLocation.Y = CurrentXY.Y * 400.f;
	EndLocation.Z = StartLocation.Z;

	//Create the batch packet
	BatchPacketToSend.Set_Batch_Packet(ControlledPawn, false, StartLocation.X, StartLocation.Y, StartLocation.Z,
		EndLocation.X, EndLocation.Y, EndLocation.Z, EnemyWalkingSpeed, TempFunctionWrapper);

	//Notify the ai manager through the GS
	myDungeonState->Notify_AI_Manager_Patrol_Batch(BatchPacketToSend);
}

void AEnemyAIController::Notify_Rotate_Enemy_By_X_Amount(float YawAdder)
{
	//Clamp, so I don't get huge added yaws. Does mean that can't have multiple rotations but for now it's fine as I
	//expect set actor rotation to pick the shortest path which basically means I have to code better rotation logic
	//Anyway if the need arises later on during the making of the game
	YawAdder = FMath::Clamp(YawAdder, -360.f, 360.f);

	//Create an error tolerance when needing to wrap the the normalized yaw from 0 - 360 range
	float Error_Tolerance = 0.5f;

	//Add the yaw with either a positive or negative adder to simulate the left and right rotations
	NormalizedYaw += YawAdder;

	//This is the Yaw that will actually set the rotation of the controlled pawn, do this because 
	//I am not reading the pawn's world rotation when checking for rotation within AI functions as
	//I have my own local rotation which dictates what is North,East etc with the NormalisedYaw variable.
	float WorldYaw = ControlledPawn->GetActorRotation().Yaw;
	float AddedWorldYaw = WorldYaw + YawAdder;

	//These are the wrappers
	if (NormalizedYaw >= 360.f - Error_Tolerance)
	{
		NormalizedYaw -= 360.f;
	}
	else if (NormalizedYaw <= 0.f - Error_Tolerance)
	{
		NormalizedYaw += 360.f;
	}

	//Set the enemy speed by getting the average speed of the enemy and times it by the magnitude of your rotation
	float TempSpeed = (EnemyRotationSpeed) * (YawAdder / 90.f);

	//If inputted negative yaw for going left, make sure to make the enemy speed positive again to not have instant speed
	//for lerp
	if (TempSpeed < 0.f)
	{
		TempSpeed *= -1;
	}

	//Forward declare the batch packet
	FAIManagerBatchPacket BatchPacketToSend;

	//bind my onfinished event
	TFunction<void()> TempFunctionWrapper = [this]()
		{
			OnFinished();
		};

	BatchPacketToSend.Set_Batch_Packet(ControlledPawn, true, 0.f, 0.f, WorldYaw, 0.f, 0.f, AddedWorldYaw,
		TempSpeed, TempFunctionWrapper);

	//Then send off to the AI manager to execute the rotation (With a batch if other AI have sent their events)
	myDungeonState->Notify_AI_Manager_Patrol_Batch(BatchPacketToSend);
}

void AEnemyAIController::OnFinished()
{
	//Debug to check if my wrapper works
	//UE_LOG(LogTemp, Warning, TEXT("EnemyAIController Finished event: %s"), *ControlledPawn->GetName());
	//UE_LOG(LogTemp, Warning, TEXT("Current Normalised Yaw = %f"), NormalizedYaw);

	if (MyCurrentAIState == ECurrent_AI_State::RotatedToMoveForward) 
	{
		MyCurrentAIState = ECurrent_AI_State::MoveForward;
		Move_Forward();
	}
	else 
	{
		Start_AI();
	}
	
}
