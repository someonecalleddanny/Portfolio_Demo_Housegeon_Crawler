// Fill out your copyright notice in the Description page of Project Settings.


#include "Entities/MC.h"


// Sets default values
AMC::AMC()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	myCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("PlayerMan"));
	myCapsule->SetupAttachment(RootComponent);

	myCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraView"));
	myCamera->SetupAttachment(myCapsule);

	myCamera->SetRelativeLocation(FVector(0.f, 0.f, Camera_Z));

	Tags.Add("MC");
}

// Called when the game starts or when spawned
void AMC::BeginPlay()
{
	//Get my dungeon state
	myDungeonState = GetWorld()->GetGameState<AGS_DungeonGeneration>();

	//Check if valid and I was not stupid to add it to the game mode base
	if (myDungeonState)
	{
		//I found that starting generation in begin play caused glitches because the game state initted later than
		//This actor, So I have a delegate within my dungeonstate that broadcasts once the 2d array is valid to be read
		//from
		myDungeonState->OnGridReady.AddDynamic(this, &AMC::Spawn_At_Center_Grid);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("NOOOT HISM GAME STATE FOUND TO DISPLAY DUNGEON GENERATION!"));
	}

	if (APlayerController* myController = Cast<APlayerController>(GetController()))
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(myController->GetLocalPlayer());

		Subsystem->AddMappingContext(myMappingContext, 0);
	}

	Super::BeginPlay();
}

void AMC::Spawn_At_Center_Grid()
{
	//Get The dungeon state
	TArray<TArray<EDungeonGenerationType>> DungeonGridInfo = myDungeonState->Get_Dungeon_Grid_Info();

	//Check if the array is valid so no crashes
	if (DungeonGridInfo.IsValidIndex(0)) 
	{
		if (DungeonGridInfo[0].IsValidIndex(0)) 
		{
			FVector Location;

			//Each cube/cell is 400x400, by getting half the array size of grid = center
			Location.X = (DungeonGridInfo.Num() / 2) * 400.f;
			Location.Y = (DungeonGridInfo[0].Num() / 2) * 400.f;
			Location.Z = 88.f;

			SetActorLocation(FVector(Location));

			//Unreal's rotation system is crap so I am brute forcing my normalised rotation for every actor spawned to face
			//0,0 of the grid which is the top left, testing I found yaw -90 faces that with my player so set it to this
			FRotator CustomRotationSpawn;
			CustomRotationSpawn.Yaw = -90;

			SetActorRotation(FQuat(CustomRotationSpawn));

			//Then update the navigation grid to notify that the center location is occupied by the player
			myGridTransform.X = DungeonGridInfo.Num() / 2;
			myGridTransform.Y = DungeonGridInfo[0].Num() / 2;

			//Create a normalised rotation, (useful for when checking navigation grid)
			myGridTransform.NormalizedYaw = 0;

			//For now, won't add logic to update walked on/off grids, that's for later with AI

			/*
				TO DO: Add the walk on/ walk off grid once AI is started
			*/
		}
		
	}
}

void AMC::MoveForward(const FInputActionValue& Value)
{
	//the whole movement logic relies on the game state, so instantly shoot red flags if not valid
	if (!myDungeonState) 
	{
		UE_LOG(LogTemp, Error, TEXT("Dungeon Game State not found!!!"));
		return;
	} 

	//Will use later to keep walking whilst walking is held down
	bKeepWalkingForward = Value.Get<bool>();


	//Since timelines create a delay in movement to the next cell check if the player tries to move again during that
	//animation, also check if the next cell to move forward is actually a movable place
	if (bAbleToMove && myDungeonState->Can_Move_Forward(myGridTransform.X, myGridTransform.Y, myGridTransform.NormalizedYaw))
	{
		UE_LOG(LogTemp, Display, TEXT("MOVING FORWARD!!!"));

		myDungeonState->Moving_Forward(myGridTransform.X, myGridTransform.Y, myGridTransform.NormalizedYaw);

		//Set to false so the timeline can do the movement animation, set to true on finished in BP
		bAbleToMove = false;

		FVector StartLocation = GetActorLocation();

		//Get the end location by getting the forward vector + start location. Don't change the z as only moving on
		//x, y axes
		FVector EndLocation = GetActorLocation() + (GetActorForwardVector() * 400.f);
		EndLocation.Z = StartLocation.Z;

		Call_Move_Forward(StartLocation, EndLocation);
	}
}

void AMC::RotateLeftRight(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Display, TEXT("Movement Left Right"));
	float LeftRightChecker = Value.Get<float>();
	FRotator Desired_Rotation = GetActorRotation();


	if (LeftRightChecker < 0.f)
	{
		UE_LOG(LogTemp, Display, TEXT("LEFT ROTATING, current rotation = %f"), GetActorRotation().Yaw);

		Desired_Rotation.Yaw -= 90.f;
		//SetActorRotation(Current_Rotation);

		if (bAbleToMove)
		{
			/*
			I tried to make a timeline in CPP but it's just way tooooo much work when I just can make it in BP
			*/
			Call_Rotate_90(GetActorRotation(), Desired_Rotation);
			bAbleToMove = false;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("CURRENTLY MOVING OR FIGHTING"));
		}

	}
	else
	{
		Desired_Rotation.Yaw += 90.f;
		//SetActorRotation(Desired_Rotation);
		if (bAbleToMove)
		{
			UE_LOG(LogTemp, Display, TEXT("Right ROTATING, current rotation = %f"), GetActorRotation().Yaw);
			/*
			I tried to make a timeline in CPP but it's just way tooooo much work when I just can make it in BP
		*/
			Call_Rotate_90(GetActorRotation(), Desired_Rotation);
			bAbleToMove = false;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("CURRENTLY MOVING OR FIGHTING"));
		}
	}
}

void AMC::Rotate180(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Display, TEXT("Movement Back"));
	FRotator Desired_Rotation = GetActorRotation();

	Desired_Rotation.Yaw += 180.f;

	if (bAbleToMove)
	{
		UE_LOG(LogTemp, Display, TEXT("Right ROTATING"));

		/*
			I tried to make a timeline in CPP but it's just way tooooo much work when I just can make it in BP
		*/
		Call_Rotate_180(GetActorRotation(), Desired_Rotation);
		bAbleToMove = false;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CURRENTLY MOVING"));
	}
}

// Called every frame
void AMC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMC::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* AddInputAction = Cast<UEnhancedInputComponent>(InputComponent))
	{
		AddInputAction->BindAction(IA_MoveForward, ETriggerEvent::Triggered, this, &AMC::MoveForward);
		AddInputAction->BindAction(IA_RotateLeftRight, ETriggerEvent::Triggered, this, &AMC::RotateLeftRight);
		AddInputAction->BindAction(IA_RotateLeftRight180, ETriggerEvent::Triggered, this, &AMC::Rotate180);
	}
}

