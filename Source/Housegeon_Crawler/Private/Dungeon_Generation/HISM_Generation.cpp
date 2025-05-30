// Fill out your copyright notice in the Description page of Project Settings.


#include "Dungeon_Generation/HISM_Generation.h"
#include "Kismet/GameplayStatics.h"


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

	//First destroy all individual Actors that were spawned
	if (myDungeonState) 
	{
		TArray<AActor*> FoundActors;

		//Get all of the actors that derive from APOI_Base_Class, The idea is that when the level is regenerated,
		//...All of the previous POI actors will have to be detroyed and made a new
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APOI_Base_Class::StaticClass(), FoundActors);

		for (AActor* Actor : FoundActors)
		{
			if (Actor)
			{
				Actor->Destroy();
			}
		}

		//Deleting mesh instances is simpler, just clear the instances of the HISM stack
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
	// If the dungeon grid array is flat out not created then return
	if (!DungeonGridInfo_PARAM.IsValidIndex(0)) return;
	if (!DungeonGridInfo_PARAM[0].IsValidIndex(0)) return;

	UE_LOG(LogTemp, Warning, TEXT("Wall Generation Started!"));

	//Create the transform that will be inputted when I add an instance to my HISM Wall
	FTransform WallTransform;

	//Forward declare location
	FVector Location(0.0f,0.0f,0.0f);

	//Forward declare rotation
	FRotator Rotation;

	//Go through each of the grid info and find which ones have the wall enum and then add instance
	for (int x = 0; x < DungeonGridInfo_PARAM.Num(); x++)
	{
		for (int y = 0; y < DungeonGridInfo_PARAM[0].Num(); y++)
		{
			WallTransform.SetLocation(Location);

			//Get a random rotation on the z from 90 degree intervals on the Yaw
			WallTransform.SetRotation(FQuat(Get_Random_Generation_Rotation()));

			//Go through a switch case to determine what to spawn
			switch (DungeonGridInfo_PARAM[x][y]) 
			{

			case EDungeonGenerationType::Wall:
				HISM_Walls->AddInstance(WallTransform, true);
				break;

			case EDungeonGenerationType::Spawn:
				//Add the spawn in the middle
				HISM_Floors->AddInstance(WallTransform, true);

				//Then, add the compass, Since I want the compass to be accurate, I will reset the rotation to 0.0f
				WallTransform.SetRotation(FQuat(FRotator(0.0f, 0.0f, 0.0f)));
				GetWorld()->SpawnActor<AActor>(CompassPOI, WallTransform);
				break;

			case EDungeonGenerationType::EndPoint:
				// Spawn the endpoint actor
				GetWorld()->SpawnActor<AActor>(EndpointPOI, WallTransform);
				break;

			case EDungeonGenerationType::ChestPOI:
				//This function will spawn an actor that will contain a wall with a gap within it
				//Since I haven't made a chest mesh yet, a HISM wall instance will have to do for the time being
				/*
					Future Danny, Create a parent POI class, then add the bp class as a UPROPERTY and then spawn actor
					from class with random rotation etc (POIs have a walkable perimeter so don't worry)
				*/
				//UE_LOG(LogTemp, Warning, TEXT("WALL FOUND TO Generate!"));
				HISM_Walls->AddInstance(WallTransform, true);
				break;

			//If another type of generation, just make it a floor
			default:
				HISM_Floors->AddInstance(WallTransform, true);
				break;
			}

			//Once finished with column, got to the next one
			Location.Y += 400.f;
		}
		Location.Y = 0.f;

		//Once finished with the row, go to the next one
		Location.X += 400.f;
	}

	//Then add the compass actor that helps the player navigate the map
	//GetWorld()->SpawnActor<AActor>(CompassPOI, WallTransform);

	//Once finished with the generation of the dungeon, now create a perimeter
	Create_Dungeon_Perimeter(DungeonGridInfo_PARAM);
}

void AHISM_Generation::Create_Dungeon_Perimeter(TArray<TArray<EDungeonGenerationType>> DungeonGridInfo_PARAM)
{
	// If the dungeon grid array is flat out not created then return
	if (!DungeonGridInfo_PARAM.IsValidIndex(0)) return;
	if (!DungeonGridInfo_PARAM[0].IsValidIndex(0)) return;

	//Create the transform that will be inputted when I add an instance to my HISM Wall
	FTransform WallTransform;

	//Forward declare location to be diagonally up and left to start encasing the dungeon in a perimeter wall
	FVector Location(-400.0f, -400.0f, 0.0f);

	//Forward declare rotation
	FRotator Rotation(0.0f,0.0f,0.0f);

	for (int i = 0; i < 2; i++) 
	{
		//Start a diagonal left from the start cell, do twice because I want to make perimeter on opposite side
		Location.X = -400.f;
		
		//Keep making walls until you reach the max size of the grid + 1
		for (int x = -1; x < DungeonGridInfo_PARAM.Num() + 1; x++) 
		{
			WallTransform.SetLocation(Location);

			//I found that making a variable stops weird rotation bugs when switching to quat for this class so ya
			Rotation = Get_Random_Generation_Rotation();

			//Get a random rotation on the z from 90 degree intervals on the Yaw
			WallTransform.SetRotation(Rotation.Quaternion());

			HISM_Walls->AddInstance(WallTransform, true);

			Location.X += 400.f;
		}
		//Go to the bottom half of the dungeon perimeter
		Location.Y = 400.f * DungeonGridInfo_PARAM[0].Num() + 1;
	}

	//Go Back to the leftmost perimeter side
	Location.X = -400.f;
	//Now I need to make the side perimeters
	for (int i = 0; i < 2; i++) 
	{
		//Start from 0 because there is a perimeter already above
		Location.Y = 0.f;

		for (int y = 0; y < DungeonGridInfo_PARAM[0].Num(); y++) 
		{
			//I found that making a variable stops weird rotation bugs when switching to quat for this class so ya
			Rotation = Get_Random_Generation_Rotation();

			WallTransform.SetLocation(Location);

			//Get a random rotation on the z from 90 degree intervals on the Yaw
			WallTransform.SetRotation(Rotation.Quaternion());

			HISM_Walls->AddInstance(WallTransform, true);

			Location.Y += 400.f;
		}

		Location.X = 400.f * DungeonGridInfo_PARAM.Num() + 1;
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

