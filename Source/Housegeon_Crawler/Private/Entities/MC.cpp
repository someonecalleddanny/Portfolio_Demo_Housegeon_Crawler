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
	Super::BeginPlay();
	
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

			SetActorLocation(FVector(Location));
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

}

