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

	//I let the BP set the dungeon viewer widget for now, I will convert once the mouse logic is finished and tested
	if (DungeonViewerWidget) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Found Dungeon Viewer Widget!"));

		DungeonViewerWidget->Call_OnMouseWheel.AddDynamic(this, &ADungeonViewer::ZoomInOut);
	}
	else 
	{
		UE_LOG(LogTemp, Error, TEXT("NOOT Found Dungeon Viewer Widget!"));
	}
}

void ADungeonViewer::Spawn_At_Center_Grid()
{
	//Get The dungeon state
	TArray<TArray<EDungeonGenerationType>> DungeonGridInfo = myDungeonState->Get_Dungeon_Grid_Info();

	//Set the default rotation for the pawn
	FRotator NewRotation;
	NewRotation.Yaw = -90.f;
	NewRotation.Pitch = -40.f;
	SetActorRotation(NewRotation);

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

