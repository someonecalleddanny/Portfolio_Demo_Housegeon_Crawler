// Fill out your copyright notice in the Description page of Project Settings.


#include "Entities/MC.h"
#include "Interfaces/POI_Interaction.h"


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

			SetRandomSpawnRotation();

			//Then update the navigation grid to notify that the center location is occupied by the player
			myGridTransform.X = DungeonGridInfo.Num() / 2;
			myGridTransform.Y = DungeonGridInfo[0].Num() / 2;

			//For now, won't add logic to update walked on/off grids, that's for later with AI

			/*
				TO DO: Add the walk on/ walk off grid once AI is started
			*/
		}
		
	}
}

void AMC::Manual_MoveForward()
{
	//Check if the forward cell (check rotation from normalised yaw, set when calling rotation functions),
	//is movable
	if (myDungeonState->Can_Move_Forward(myGridTransform.X, myGridTransform.Y, myGridTransform.NormalizedYaw))
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
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Forward path blocked, can't move forward"))
	}
}

void AMC::MoveForward(const FInputActionValue& Value)
{
	//Used for BP timelines, once forward lerp is finished, if the move forward key is still pressed, it will call movement
	//again, This does assume that the IA class has the OnPressed + OnReleased trigger
	bKeepWalkingForward = Value.Get<bool>();

	//the whole movement logic relies on the game state, so instantly shoot red flags if not valid
	if (!myDungeonState) 
	{
		UE_LOG(LogTemp, Error, TEXT("Dungeon Game State not found!!!"));
		return;
	} 

	//if already moving from another function or this, instantly return and don't do logic below
	if (!bAbleToMove)
	{
		UE_LOG(LogTemp, Warning, TEXT("CURRENTLY MOVING OR FIGHTING"));
		return;
	}

	//Call the C++ logic to move forward
	Manual_MoveForward();
}

void AMC::RotateLeftRight(const FInputActionValue& Value)
{

	//if already moving from another function or this, instantly return and don't do logic below
	if (!bAbleToMove)
	{
		UE_LOG(LogTemp, Warning, TEXT("CURRENTLY MOVING OR FIGHTING"));
		return;
	}

	UE_LOG(LogTemp, Display, TEXT("Movement Left Right"));
	float LeftRightChecker = Value.Get<float>();
	FRotator Desired_Rotation = GetActorRotation();

	//Rotate -90 (left)
	if (LeftRightChecker < 0.f)
	{
		UE_LOG(LogTemp, Display, TEXT("LEFT ROTATING, current rotation = %f"), GetActorRotation().Yaw);

		//The temp normalised yaw system to make sure that the grid navigation is working (will fix on later stage of
		//development, AI system)
		myGridTransform.NormalizedYaw -= 90.f;

		//if started on the forward rotation but moved left, go to WEST which is 270.f,
		// do this to force 4 yaw values 0, 90 , 180, 270
		if (myGridTransform.NormalizedYaw < 0) 
		{
			myGridTransform.NormalizedYaw = 270.f;
		}

		//Rotate along the yaw to the left
		Desired_Rotation.Yaw -= 90.f;
		/*
		I tried to make a timeline in CPP but it's just way tooooo much work when I just can make it in BP
		*/
		Call_Rotate_90(GetActorRotation(), Desired_Rotation);
		bAbleToMove = false;

		//Rotate the normalised yaw left for the grid naviation system (Currently a temp solution will come back later)


	}
	else//Rotating 90 degrees right
	{
		//The temp normalised yaw system to make sure that the grid navigation is working (will fix on later stage of
		//development, AI system)
		myGridTransform.NormalizedYaw += 90.f;
		
		//if on WEST (270.f) degrees on yaw, Set it to NORTH which is 0.f, do this to force 4 yaw values 0, 90 , 180, 270
		if (myGridTransform.NormalizedYaw > 270.f)
		{
			myGridTransform.NormalizedYaw = 0.f;
		}

		//Rotate along the yaw to rotate right
		Desired_Rotation.Yaw += 90.f;

		UE_LOG(LogTemp, Display, TEXT("Right ROTATING, current rotation = %f"), GetActorRotation().Yaw);
		/*
		I tried to make a timeline in CPP but it's just way tooooo much work when I just can make it in BP
		*/
		Call_Rotate_90(GetActorRotation(), Desired_Rotation);
		bAbleToMove = false;
	}
}

void AMC::Rotate180(const FInputActionValue& Value)
{
	//if already moving from another function or this, instantly return and don't do logic below
	if (!bAbleToMove)
	{
		UE_LOG(LogTemp, Warning, TEXT("CURRENTLY MOVING OR FIGHTING"));
		return;
	}

	UE_LOG(LogTemp, Display, TEXT("Movement Back"));
	FRotator Desired_Rotation = GetActorRotation();

	UE_LOG(LogTemp, Display, TEXT("Right ROTATING"));
	//Rotate along the Yaw to rotate the player behind
	Desired_Rotation.Yaw += 180.f;

	//Do the Navigation grid transform
	myGridTransform.NormalizedYaw += 180.f;

	//If goes over 270, normalise by subtrating by 360
	if (myGridTransform.NormalizedYaw > 270.f)
	{
		myGridTransform.NormalizedYaw -= 360.f;
	}

	/*
		I tried to make a timeline in CPP but it's just way tooooo much work when I just can make it in BP
	*/
	Call_Rotate_180(GetActorRotation(), Desired_Rotation);
	bAbleToMove = false;
}

void AMC::Interacted(const FInputActionValue& Value)
{
	//Get the player controller
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();

	FMinimalViewInfo CamInfo;

	myCamera->GetCameraView(0.f, CamInfo);

	FVector WorldLocation = CamInfo.Location;

	FVector EndLocation = CamInfo.Rotation.Vector();

	EndLocation.Normalize();

	FVector LineTraceEnd = WorldLocation + (EndLocation * 500.f);

	//Passed as a reference so has to be created here
	FHitResult HitResult;
	//same here
	FCollisionQueryParams CollisionParams;

	//ignore the fact that the line trace might hit the self actor
	CollisionParams.AddIgnoredActor(this);

	//The actual line trace being created
	if (GetWorld()->LineTraceSingleByChannel(HitResult, WorldLocation, LineTraceEnd,
		ECollisionChannel::ECC_Visibility, CollisionParams))
	{
		DrawDebugLine(GetWorld(), WorldLocation,
			LineTraceEnd, FColor::Red, false, 2.f, 0, 0.1f);

		if (HitResult.GetActor()) 
		{
			if (HitResult.GetActor()->ActorHasTag("POI")) 
			{
				IPOI_Interaction* POIInterface = Cast<IPOI_Interaction>(HitResult.GetActor());

				if (POIInterface) 
				{
					POIInterface->Interacted();
				}
			}
		}
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
		AddInputAction->BindAction(IA_Interacted, ETriggerEvent::Triggered, this, &AMC::Interacted);
	}
}

void AMC::SetRandomSpawnRotation()
{
	//Unreal's rotation system is crap so I am brute forcing my normalised rotation for every actor spawned to face
	//0,0 of the grid which is the top left, testing I found yaw -90 faces that with my player so set it to this
	//Once I am comfortable with the navigation working, I will come back to this when I figure out how to 
	//translate relative rotations with 0 being forward on the navigation grid (This will be priority when coding
	//AI)
	FRotator CustomRotationSpawn (0.0f, 0.0f, 0.0f);
	

	int RandSpawn = FMath::RandRange(0, 3);

	switch (RandSpawn) 
	{
	case 0:
		CustomRotationSpawn.Yaw = -90;
		//Create a normalised rotation, (useful for when checking navigation grid), 0 means forward
		myGridTransform.NormalizedYaw = 0.f;

	case 1:
		CustomRotationSpawn.Yaw = 0.0f;
		//Create a normalised rotation, (useful for when checking navigation grid), 90 means right
		myGridTransform.NormalizedYaw = 90.f;

	case 2:
		CustomRotationSpawn.Yaw = 90.0f;
		//Create a normalised rotation, (useful for when checking navigation grid), 0 means back
		myGridTransform.NormalizedYaw = 180.f;

	case 3:
		CustomRotationSpawn.Yaw = 180.0f;
		//Create a normalised rotation, (useful for when checking navigation grid), 0 means back
		myGridTransform.NormalizedYaw = 270.f;
	}

	SetActorRotation(FQuat(CustomRotationSpawn));
}

