// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/EnemyAIController.h"

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	//Get my dungeon state
	myDungeonState = GetWorld()->GetGameState<AGS_DungeonGeneration>();

	//Check if valid and I was not stupid to add it to the game mode base
	if (myDungeonState)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnemyAIController possessed: %s"), *InPawn->GetName());

		SpawnedEnemy();

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

	if (GetPawn()) 
	{
		UE_LOG(LogTemp, Display, TEXT("The enemy has indeed been possessed!"));
		SetRandomRotation();
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

	GetPawn()->SetActorRotation(FQuat(CustomRotationSpawn));
	//SetControlRotation(CustomRotationSpawn);
}

void AEnemyAIController::Start_AI()
{
	//Default to the check player AI state when starting or restarting the AI, Don't need timer as I want to be fast to do
	//Event
	MyCurrentAIState = ECurrent_AI_State::CheckPlayer;
	Check_MC();
}

void AEnemyAIController::AI_Next_State(float Delay)
{
	//If the coder accidently or intentionaly wants a default time until the next AI state happens, set the delay to not be
	//Instant (Stops stack overflow)
	if (Delay <= 0.f) 
	{
		UE_LOG(LogTemp, Warning, TEXT("AI Next state had a delay of <= 0.f"));
		Delay = 0.01f;
	}

	//Then call the timer manager to have a delay until doing the next function from the current AI state selected
	GetWorldTimerManager().SetTimer(TH_NextAIEvent, this, &AEnemyAIController::TimerEvent_DO_State_Function, Delay, false);
}

void AEnemyAIController::TimerEvent_DO_State_Function()
{
	/*
		Here I create a pointer void function that is found from the myAIStates map. The key/identifiers
		are enums that link to void pointers for this class instance. This is basically my version of creating
		a blackboard key when you create a behaviour tree.
	*/
	void (AEnemyAIController:: * AI_State_Call)() = myAIStates.FindRef(MyCurrentAIState);

	//Check if the void pointer is valid
	if (AI_State_Call)
	{
		/*
			Here I call the void pointer. First I have to encase this->*AI_State_Call due to order of operation.
			But basically I get the current actor reference and then access the current function pointer which
			was made above
		*/
		(this->*AI_State_Call)();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("NOOOT Found The AI State to Play for Enemy"));
	}
}

void AEnemyAIController::Check_MC()
{
	bool bFoundPlayer = false;

	if (bFoundPlayer) 
	{
		/*
			Will need to have a checker to check where the player is but for now just do patrolling AI behaviour
		*/
	}
	else 
	{
		MyCurrentAIState = ECurrent_AI_State::RandomPatrolMovementSelector;
	}
	
	AI_Next_State(0.1f);
}

void AEnemyAIController::Choose_Random_Patrol()
{
	int RandomInt = FMath::RandRange(0, 3);
	//Debugging
	RandomInt = 0;

	switch (RandomInt) 
	{
	case 0 :
		MyCurrentAIState = ECurrent_AI_State::MoveForward;
		break;
	default:
		break;
	}

	//Choose next state, this function has a default delay of 0.01f if lazy to specifiy a quick next state
	AI_Next_State();
}
