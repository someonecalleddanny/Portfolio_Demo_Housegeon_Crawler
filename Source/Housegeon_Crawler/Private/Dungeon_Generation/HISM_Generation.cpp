// Fill out your copyright notice in the Description page of Project Settings.


#include "Dungeon_Generation/HISM_Generation.h"


// Sets default values
AHISM_Generation::AHISM_Generation()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	HISM_Walls = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("Walls"));
	HISM_Walls->SetupAttachment(RootComponent);

	HISM_Floors = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("Floors"));
	HISM_Floors->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AHISM_Generation::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("Dungeon HISM generation started!"));

	//Get my dungeon state
	myDungeonState = GetWorld()->GetGameState<AGS_DungeonGeneration>();

	//Check if valid and I was not stupid to add it to the game mode base
	if (myDungeonState)
	{
		//I found that starting generation in begin play caused glitches because the game state initted later than
		//This actor, So I have a delegate within my dungeonstate that broadcasts once the 2d array is valid to be read
		//from
		myDungeonState->OnGridReady.AddDynamic(this, &AHISM_Generation::Start_Generation);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("NOOOT HISM GAME STATE FOUND TO DISPLAY DUNGEON GENERATION!"));
	}
}

void AHISM_Generation::Start_Generation()
{
	//Start the dungeon gen
	if (myDungeonState) 
	{
		HISM_Walls->ClearInstances();
		HISM_Floors->ClearInstances();

		UE_LOG(LogTemp, Warning, TEXT("HISM GAME STATE FOUND TO DISPLAY DUNGEON GENERATION!"));
		Display_Everything_From_Dungeon_Grid(myDungeonState->Get_Dungeon_Grid_Info());

		//Thinking of doing multiple dungeon gens in one session
		//myDungeonState->OnGridReady.RemoveDynamic(this, &AHISM_Generation::Start_Generation);
	}
}

// Called every frame
void AHISM_Generation::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AHISM_Generation::Display_Everything_From_Dungeon_Grid(TArray<TArray<EDungeonGenerationType>> DungeonGridInfo_PARAM)
{
	UE_LOG(LogTemp, Warning, TEXT("Wall Generation Started!"));

	//Create the transform that will be inputted when I add an instance to my HISM Wall
	FTransform WallTransform;

	//Forward declare location
	FVector Location(0.0f,0.0f,0.0f);

	//Forward declare rotation
	FRotator Rotation;

	if (DungeonGridInfo_PARAM.IsValidIndex(0)) 
	{
		UE_LOG(LogTemp, Warning, TEXT("WALL X VALID"));
		if (DungeonGridInfo_PARAM[0].IsValidIndex(0)) 
		{
			UE_LOG(LogTemp, Warning, TEXT("WALL Y VALID"));
			//Go through each of the grid info and find which ones have the wall enum and then add instance
			for (int x = 0; x < DungeonGridInfo_PARAM.Num(); x++)
			{
				for (int y = 0; y < DungeonGridInfo_PARAM[0].Num(); y++)
				{
					//Check if the dungeon grid returns a wall enum type to generate a wall
					if (DungeonGridInfo_PARAM[x][y] == EDungeonGenerationType::Wall)
					{
						//UE_LOG(LogTemp, Warning, TEXT("WALL FOUND TO Generate!"));
						WallTransform.SetLocation(Location);

						//Get a random rotation on the z from 90 degree intervals on the Yaw
						WallTransform.SetRotation(FQuat(Get_Random_Generation_Rotation()));

						HISM_Walls->AddInstance(WallTransform, true);
					}
					else if (DungeonGridInfo_PARAM[x][y] == EDungeonGenerationType::Floor ||
						DungeonGridInfo_PARAM[x][y] == EDungeonGenerationType::Spawn ||
						DungeonGridInfo_PARAM[x][y] == EDungeonGenerationType::EndPoint)
					{
						//UE_LOG(LogTemp, Warning, TEXT("Floor FOUND TO Generate!"));
						WallTransform.SetLocation(Location);

						//Get a random rotation on the z from 90 degree intervals on the Yaw
						WallTransform.SetRotation(FQuat(Get_Random_Generation_Rotation()));

						HISM_Floors->AddInstance(WallTransform, true);
					}
					{
						//UE_LOG(LogTemp, Error, TEXT("NO WALL FOUND TO Generate!"));
					}
					//Once finished with column, got to the next one
					Location.Y += 400.f;
				}
				Location.Y = 0.f;

				//Once finished with the row, go to the next one
				Location.X += 400.f;
			}
		}
		else 
		{
			UE_LOG(LogTemp, Error, TEXT("WALL Y NOT VALID"));
		}
	}
	else 
	{
		UE_LOG(LogTemp, Error, TEXT("WALL X NOT VALID"));
	}
}

FRotator AHISM_Generation::Get_Random_Generation_Rotation()
{
	FRotator RandomRotation;

	int Random = FMath::RandRange(0, 3);

	switch (Random) 
	{
	case 0:
		RandomRotation.Yaw = 0.0f;
		break;

	case 1:
		RandomRotation.Yaw = 90.0f;
		break;

	case 2:
		RandomRotation.Yaw = 180.0f;
		break;

	case 3:
		RandomRotation.Yaw = 270.0f;
		break;

	default:
		RandomRotation.Yaw = 0.0f;
		break;
	}

	return RandomRotation;
}

