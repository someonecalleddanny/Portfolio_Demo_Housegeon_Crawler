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

	//When the player switches to the dungeon viewer pawn (this), I want to reset the camera transform to default
	Reset_Camera();

	//I let the BP set the dungeon viewer widget for now, I will convert once the mouse logic is finished and tested
	if (DungeonViewerWidget) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Found Dungeon Viewer Widget!"));

		//I bind the delegate on possessed because the widget will be destroyed once unpossessed for the player, so need
		//to rebind, once the widget is destroyed all bound functions will be destroyed
		DungeonViewerWidget->Call_OnMouseWheel.AddDynamic(this, &ADungeonViewer::ZoomInOut);
		DungeonViewerWidget->Call_OnMouseDrag.AddDynamic(this, &ADungeonViewer::RotateCameraFromWidget);
		DungeonViewerWidget->Call_OnResetCamera.AddDynamic(this, &ADungeonViewer::Reset_Camera);
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

	//Now Set the camera using information from the game state

	//Get The dungeon state
	TArray<TArray<EDungeonGenerationType>> DungeonGridInfo = myDungeonState->Get_Dungeon_Grid_Info();

	//Check if the array is valid so no crashes
	if (DungeonGridInfo.IsValidIndex(0))
	{
		if (DungeonGridInfo[0].IsValidIndex(0))
		{
			//Old code: For some reason, using this in Reset_Camera and using the delegate to reset the camera would
			// fudge the location of the camera so use a private property instead called defaultcameralocation
			//FVector Location;

			//Each cube/cell is 400x400, by getting half the array size of grid = center
			DefaultCameraLocation.X = (DungeonGridInfo.Num() / 2) * 400.f;
			DefaultCameraLocation.Y = (DungeonGridInfo[0].Num() / 2) * 400.f;

			//Set the zoom of the camera to fit the entire grid, the DefaultLength_Multiplier is based on trial and error
			//to see what looks the best
			CurrentCameraLength = DefaultLength_Multiplier * (DungeonGridInfo.Num());
			Camera_DefaultLength = DefaultLength_Multiplier * (DungeonGridInfo.Num());

			mySpringArm->TargetArmLength = CurrentCameraLength;

			SetActorLocation(FVector(DefaultCameraLocation));
		}
	}
}

void ADungeonViewer::ZoomInOut(float MouseWheelDelta)
{
	//If the button to reset the camera is not visible in the widget, then make it visible as just changed camera transform
	if (!bShowResetCamera)
	{
		bShowResetCamera = true;
		DungeonViewerWidget->SetResetCameraVisibility(bShowResetCamera);
	}

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
	
	//If the button to reset the camera is not visible in the widget, then make it visible as just changed camera transform
	if (!bShowResetCamera) 
	{
		bShowResetCamera = true;
		DungeonViewerWidget->SetResetCameraVisibility(bShowResetCamera);
	}

	AddControllerYawInput(DeltaX * CameraYawSensitivity);

	float Pitch = GetControlRotation().Pitch;

	// Normalise the control rotation pitch to only be within a positive bound
	if (Pitch > 180.f)
	{
		Pitch -= 360.f;
	}

	//Then... Check if pitch is less than min pitch (A negative value allows the camera to go up)
	if (Pitch < Min_Pitch)
	{
		AddControllerPitchInput(DeltaY * CameraPitchSensitivity);
	}
	else if (DeltaY > 0.0f) //If below the min, Only allow a positive y delta so that the camera can move up
	{
		AddControllerPitchInput(DeltaY * CameraPitchSensitivity);
	}

	//Once again have to normalise the pitch
	Pitch = GetControlRotation().Pitch;

	// Normalise the control rotation pitch to only be within a positive bound
	if (Pitch > 180.f)
	{
		Pitch -= 360.f;
	}
	//Clamp the pitch so that mouse acceleration cannot go further than the min pitch (from the mouse y delta)
	Pitch = FMath::Clamp(Pitch, -90.f, Min_Pitch);

	//Create a normalised rotator to add the clamped pitch to the control rotation
	FRotator NormalisedRotation = GetControlRotation();
	NormalisedRotation.Pitch = Pitch;

	//Then, set the control rotation from the normalisedrotation with the clamped pitch
	GetController()->SetControlRotation(NormalisedRotation);
}

void ADungeonViewer::Reset_Camera()
{
	//If the button to reset the camera is visible in the widget, then make it collapsed
	if (bShowResetCamera)
	{
		bShowResetCamera = false;
		DungeonViewerWidget->SetResetCameraVisibility(bShowResetCamera);
	}

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

	//Set the actor to the middle of the grid by using a private property which is set when the dungeon is generated
	SetActorLocation(FVector(DefaultCameraLocation));

	//Reset the zoom by setting the current length to the default length, do this because when you scroll it sets
	//the current length to a value to set for the spring arm. Lamens terms, it current length needs to be reset because
	//it is changed elsewhere. Then, set the springarm to the new default current length
	CurrentCameraLength = Camera_DefaultLength;
	mySpringArm->TargetArmLength = CurrentCameraLength;
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

