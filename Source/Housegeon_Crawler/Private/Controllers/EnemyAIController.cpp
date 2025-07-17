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

	myDungeonState->Register_Entity_Cell_Location(CurrentXY, ControlledPawn);
}

FIntPoint AEnemyAIController::GetCurrentXY()
{
	return CurrentXY;
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
		//In world -90 points to North
		CustomRotationSpawn.Yaw = -90;
		//The setting of the enum looks weird but I encapsulated my enum within my struct mainly for the enum size in
		//rotation wrapping which I don't want anything else to intefere with
		CurrentCompassDirection.Set_Compass_Direction(FCompassDirection::ECompassDirection::North);
		break;

	case 1:
		//In world 0.0 points to East
		CustomRotationSpawn.Yaw = 0.0f;
		CurrentCompassDirection.Set_Compass_Direction(FCompassDirection::ECompassDirection::East);
		break;

	case 2:
		//In world 90.0 points to South
		CustomRotationSpawn.Yaw = 90.0f;
		CurrentCompassDirection.Set_Compass_Direction(FCompassDirection::ECompassDirection::South);
		break;

	case 3:
		//In world 180.0 points to West
		CustomRotationSpawn.Yaw = 180.0f;
		CurrentCompassDirection.Set_Compass_Direction(FCompassDirection::ECompassDirection::West);
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
	//RandomInt = 0;

	switch (RandomInt) 
	{
	case 0 :
		MyCurrentAIState = ECurrent_AI_State::MoveForward;
		Notify_Delayed_Move_Forward();
		break;

	case 1:
		MyCurrentAIState = ECurrent_AI_State::Rotate90;
		Rotate_Enemy_By_X_Amount(90.f, true);
		break;

	case 2:
		MyCurrentAIState = ECurrent_AI_State::RotateMinus90;
		Rotate_Enemy_By_X_Amount(-90.f, true);
		break;

	case 3:
		MyCurrentAIState = ECurrent_AI_State::Rotate180;
		Rotate_Enemy_By_X_Amount(180.f, true);
		break;

	default:
		UE_LOG(LogTemp, Error, TEXT("Your random patrol index is invalid within your ai controller"));
		break;
	}
}

void AEnemyAIController::Notify_Delayed_Move_Forward()
{
	//Create the functor that binds the OnDelayedMoveForward member function held within this class
	TFunction<FAIManagerBatchPacket()> BoundDelayedBatchFunction = [this]()
		{
			return OnDelayedMoveForward();
		};

	//Add the created functor to the struct, this will be called when the ai manager first pops the batched struct
	//which will call the OnDelayedMoveForward function here to instantly call the frame perfect logic that needs to be
	//called
	BatchPacketToSend.Set_Delayed_Batch_Packet(ControlledPawn, BoundDelayedBatchFunction);

	myDungeonState->Notify_AI_Manager_Patrol_Batch(BatchPacketToSend);
}

FAIManagerBatchPacket AEnemyAIController::Rotate_Enemy_By_X_Amount(float YawAdder, bool bNotifyBatch)
{
	//Clamp, so I don't get huge added yaws. Does mean that can't have multiple rotations but for now it's fine as I
	//expect set actor rotation to pick the shortest path which basically means I have to code better rotation logic
	//Anyway if the need arises later on during the making of the game
	YawAdder = FMath::Clamp(YawAdder, -360.f, 360.f);

	//Add the yaw with either a positive or negative adder to simulate the left and right rotations, remember need to
	//represent the internal rotations for movement as well as rotation in world (This is for internal rotation logic)
	CurrentCompassDirection.Rotate_By_X_Amount(YawAdder);

	//This is the Yaw that will actually set the rotation of the controlled pawn, do this because 
	//I am not reading the pawn's world rotation when checking for rotation within AI functions as
	//I have my own local rotation which dictates what is North,East etc with the NormalisedYaw variable.
	float WorldYaw = ControlledPawn->GetActorRotation().Yaw;
	float AddedWorldYaw = WorldYaw + YawAdder;

	//Set the enemy speed by getting the average speed of the enemy and times it by the magnitude of your rotation
	float TempSpeed = (EnemyRotationSpeed) * (YawAdder / 90.f);

	//If inputted negative yaw for going left, make sure to make the enemy speed positive again to not have instant speed
	//for lerp
	if (TempSpeed < 0.f)
	{
		TempSpeed *= -1;
	}

	//bind my onfinished event
	CurrentBoundOnFinishedFunction = [this]()
		{
			OnFinished();
		};

	BatchPacketToSend.Set_Batch_Packet(ControlledPawn, true, 0.f, 0.f, WorldYaw, 0.f, 0.f, AddedWorldYaw,
		TempSpeed, CurrentBoundOnFinishedFunction);

	if (bNotifyBatch) 
	{
		//Then send off to the AI manager to execute the rotation (With a batch if other AI have sent their events)
		myDungeonState->Notify_AI_Manager_Patrol_Batch(BatchPacketToSend);
	}
	
	return BatchPacketToSend;
}

void AEnemyAIController::OnFinished()
{
	//Debug to check if my wrapper works
	//UE_LOG(LogTemp, Warning, TEXT("EnemyAIController Finished event: %s"), *ControlledPawn->GetName());
	//UE_LOG(LogTemp, Warning, TEXT("Current Normalised Yaw = %f"), NormalizedYaw);

	//If here, the delayed function has finished executing, so flush it for a potential new delayed function to be bound
	BatchPacketToSend.Finished_OnDelayed_Function();
	BatchPacketToSend.Set_Batch_Packet_Finished();

	if (MyCurrentAIState == ECurrent_AI_State::RotatedToMoveForward) 
	{
		MyCurrentAIState = ECurrent_AI_State::MoveForward;
		Notify_Delayed_Move_Forward();
	}
	else 
	{
		Start_AI();
	}
	
}

FAIManagerBatchPacket AEnemyAIController::OnDelayedMoveForward()
{
	//Check if the enemy can actually move forward
	if (!myDungeonState->Can_Move_Forward(CurrentXY.X, CurrentXY.Y, CurrentCompassDirection))
	{
		//I cannot move forward, so I am going to find a random possible rotation that I can move forward from and do the
		//rotation for the current batch and on finish, try the Notify_Delayed_Move_Forward() function again
		return Rotate_To_Be_Able_To_Move_Forward();
	}

	//Notify to the GS that the cell that the enemy moved to is currently not movable and set the cell moved from to
	//be movable (Also sets the internal x and y cell locations to be where move forward is from the navigation grid)
	myDungeonState->Moving_Forward(ControlledPawn, CurrentXY.X, CurrentXY.Y, CurrentCompassDirection);

	//Bind your onfinished function
	CurrentBoundOnFinishedFunction = [this]()
		{
			OnFinished();
		};

	//create the start and end for the lerp
	FVector StartLocation = ControlledPawn->GetActorLocation();
	FVector EndLocation;

	//Create the end location for the lerp (Don't really need to set z but better to be safer than sorry if Unreal does
	//Unreal things to uninitted variables
	EndLocation.X = CurrentXY.X * 400.f;
	EndLocation.Y = CurrentXY.Y * 400.f;
	EndLocation.Z = StartLocation.Z;

	//Create the batch packet
	BatchPacketToSend.Set_Batch_Packet(ControlledPawn, false, StartLocation.X, StartLocation.Y, StartLocation.Z,
		EndLocation.X, EndLocation.Y, EndLocation.Z, EnemyWalkingSpeed, CurrentBoundOnFinishedFunction);

	return BatchPacketToSend;
}

FAIManagerBatchPacket AEnemyAIController::Rotate_To_Be_Able_To_Move_Forward()
{
	//Right, so if I cannot find a place to move forward to, keep choosing a random adder yaw until finding which yaw
	//The enemy can move forward on, return the updated batch packet (for move forward function to rotate instead and then
	//try move forward on next batch)
	TArray<float> RandomFloatsToChoose;
	int RandIndex;
	float TempYaw;
	float PotentialYaw;

	//Loop 3 times because the game only supports 4 yaw rotations (your current yaw could not move so need to check the
	//other 3 if they are movable, there is a chance that the AI can be boxed in from walls and other AI blocking
	//paths but I will deal with that below)
	for (int i = 0; i < PossibleAdderYawsToMoveForward.Num(); i++)
	{
		//Get a temp yaw from the possible adder yaws that the player can move forward from
		TempYaw = PossibleAdderYawsToMoveForward[i];
		PotentialYaw = TempYaw + NormalizedYaw;

		//Wrap the potential yaw to only have a value of 0 - 360 (- error tolerance) Could use FMod but I really want
		//to aggressively make this as optimised as possible, I like saving the small overhead of not using the FMath
		//class
		if (PotentialYaw >= 360.f - RotationErrorTolerance)
		{
			PotentialYaw -= 360.f;
		}
		else if (PotentialYaw <= 0.f - RotationErrorTolerance)
		{
			PotentialYaw += 360.f;
		}

		//If I found a suitable location in which I can move forward to, add it to the random floats for the enemy to
		//rotate to
		if (myDungeonState->Can_Move_Forward(CurrentXY.X, CurrentXY.Y, PotentialYaw))
		{
			RandomFloatsToChoose.Add(TempYaw);
		}
	}

	//The adder yaw that you will have to add to the normalised yaw to rotate the enemy
	float RandomAdderYaw;

	//First check if the random floats conatiner is not empty 
	if (!RandomFloatsToChoose.IsEmpty())
	{
		RandIndex = FMath::RandRange(0, RandomFloatsToChoose.Num() - 1);
		RandomAdderYaw = RandomFloatsToChoose[RandIndex];
	}
	else
	{
		//if the for loop could not find any yaw locations that the enemy could move from, means that it is stuck and
		//just needs to wait until the way is clear (i.e you are not adding rotation)
		/*
			This does mean that the rotation event finishes on the same tick but will need to check if it makes stuttering
		*/
		RandomAdderYaw = 0.f;
		UE_LOG(LogTemp, Warning, TEXT("Enemy Is boxed in, going to just stay still for this tick cycle!"));
	}

	//Change the current ai state to rotatedtomoveforward, because on finish it will call this move forward
	//function again to check if the enemy can move forward
	MyCurrentAIState = ECurrent_AI_State::RotatedToMoveForward;

	return Rotate_Enemy_By_X_Amount(RandomAdderYaw, false);
}
