// Fill out your copyright notice in the Description page of Project Settings.


#include "Entities/DungeonViewer.h"

// Sets default values
ADungeonViewer::ADungeonViewer()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	mySpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	mySpringArm->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ADungeonViewer::BeginPlay()
{
	Super::BeginPlay();
	
	//Get my dungeon state
	myDungeonState = GetWorld()->GetGameState<AGS_DungeonGeneration>();

	//Check if valid and I was not stupid to add it to the game mode base
	if (myDungeonState)
	{
		//I found that starting generation in begin play caused glitches because the game state initted later than
		//This actor, So I have a delegate within my dungeonstate that broadcasts once the 2d array is valid to be read
		//from
		myDungeonState->OnGridReady.AddDynamic(this, &ADungeonViewer::Spawn_At_Center_Grid);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("NOOOT HISM GAME STATE FOUND TO DISPLAY DUNGEON GENERATION!"));
	}
}

void ADungeonViewer::PossessedBy(AController* NewController)
{
	//Let BP handle it for now because will have to translate a lot of button logic
	Super::PossessedBy(NewController);

	Reset_Camera();

	//I let the BP set the dungeon viewer widget for now, I will convert once the mouse logic is finished and tested
	if (DungeonViewerWidget) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Found Dungeon Viewer Widget!"));

		//I bind the delegate on possessed because the widget will be destroyed once unpossessed for the player, so need
		//to rebind, once the widget is destroyed all bound functions will be destroyed
		DungeonViewerWidget->Call_OnMouseWheel.AddDynamic(this, &ADungeonViewer::ZoomInOut);
		DungeonViewerWidget->Call_OnMouseDrag.AddDynamic(this, &ADungeonViewer::RotateCameraFromWidget);
	}
	else 
	{
		UE_LOG(LogTemp, Error, TEXT("NOOT Found Dungeon Viewer Widget!"));
	}
}

void ADungeonViewer::Spawn_At_Center_Grid()
{
	//Set the default rotation for the pawn
	Reset_Camera();

	//Get The dungeon state
	TArray<TArray<EDungeonGenerationType>> DungeonGridInfo = myDungeonState->Get_Dungeon_Grid_Info();

	//Set the default rotation for the pawn
	/*
	FRotator NewRotation;
	NewRotation.Yaw = Reset_Yaw;
	NewRotation.Pitch = Reset_Pitch;
	SetActorRotation(NewRotation);
	*/
	
	//Check if the array is valid so no crashes
	if (DungeonGridInfo.IsValidIndex(0))
	{
		if (DungeonGridInfo[0].IsValidIndex(0))
		{
			FVector Location;

			//Each cube/cell is 400x400, by getting half the array size of grid = center
			Location.X = (DungeonGridInfo.Num() / 2) * 400.f;
			Location.Y = (DungeonGridInfo[0].Num() / 2) * 400.f;
			
			//Set the zoom of the camera to fit the entire grid, the DefaultLength_Multiplier is based on trial and error
			//to see what looks the best
			CurrentCameraLength = DefaultLength_Multiplier * (DungeonGridInfo.Num());
			Camera_DefaultLength = DefaultLength_Multiplier * (DungeonGridInfo.Num());

			mySpringArm->TargetArmLength = CurrentCameraLength;

			SetActorLocation(FVector(Location));
		}
	}
}

void ADungeonViewer::ZoomInOut(float MouseWheelDelta)
{
	//Zoom in/out depending on if the mouse wheel goes up or down, the mouse wheel delta is read from the dungeon viewer
	//widget that has a delegate that is broadcasted everytime the mouse wheel moves which this function is bound to
	//I minus the length when zooming in because that makes the camera shorten the distance to the center position
	(MouseWheelDelta > 0.0f) ? CurrentCameraLength -= Camera_ZoomMultiplier : CurrentCameraLength += Camera_ZoomMultiplier;

	//Clamp the length so it can't go too far in or out
	CurrentCameraLength = FMath::Clamp(CurrentCameraLength, Min_DefaultLength,
		Camera_DefaultLength + Max_DefaultLength);

	//Set the new target arm length
	mySpringArm->TargetArmLength = CurrentCameraLength;
}

void ADungeonViewer::RotateCameraFromWidget(float DeltaX, float DeltaY)
{
	/*
	FRotator NewRotation = GetActorRotation();
	NewRotation.Roll = 0;
	//UE_LOG(LogTemp, Warning, TEXT("Rotating Camera: DeltaX = %f"), DeltaX);

	//If the mouse is moving to the right, I want the the camera to move to the left (Inverted Camera View)
	if (DeltaX > 0.0f) 
	{
		NewRotation.Yaw += CameraYawRotatorValue;
	}
	//I do an else if for checking left mouse because there is a possibility of the deltaX being 0, so don't want to keep
	//Moving camera on Yaw
	else if (DeltaX < 0.0f) 
	{
		NewRotation.Yaw -= CameraYawRotatorValue;
	}

	//Same idea as the yaw but moving the camera up and down when the mouse is dragged
	if (DeltaY < 0.0f) 
	{
		NewRotation.Pitch += CameraPitchRotatorValue;
	}
	else if (DeltaY > 0.0f) 
	{
		NewRotation.Pitch -= CameraPitchRotatorValue;
	}

	//Check if the Yaw goes >/< 360/-360 because Unreal does not reset the values
	if (NewRotation.Yaw > 360.f) NewRotation.Yaw -= 360.f;
	else if (NewRotation.Yaw < -360.f) NewRotation.Yaw += 360.f;

	//Check if the Pitch goes >/< 360/-360 because Unreal does not reset the values
	if (NewRotation.Pitch > 360.f) NewRotation.Pitch -= 360.f;
	else if (NewRotation.Pitch < -360.f) NewRotation.Pitch += 360.f;

	SetActorRotation(NewRotation);
	*/
	
	AddControllerYawInput(DeltaX * CameraYawSensitivity);
	AddControllerPitchInput(DeltaY * CameraPitchSensitivity);
}

void ADungeonViewer::Reset_Camera()
{
	UE_LOG(LogTemp, Warning, TEXT("Resetting Camera!"));

	//Sets the default camera position after the player mingles with it, Use UPROPERTIES to reset the Yaw and Pitch
	FRotator ResetRotation;
	ResetRotation.Yaw = Reset_Yaw;
	ResetRotation.Pitch = Reset_Pitch;

	//The camera component uses pawn control rotation to bypass gimbal lock to have smooth camera movement
	if (GetController()) 
	{
		GetController()->SetControlRotation(ResetRotation);

		//I set the actor rotation as well in case the camera subsystem lags behind when possessed
		SetActorRotation(ResetRotation.Quaternion());
	}
}

// Called every frame
void ADungeonViewer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ADungeonViewer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

