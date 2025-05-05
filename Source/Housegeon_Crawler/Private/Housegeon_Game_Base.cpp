// Fill out your copyright notice in the Description page of Project Settings.


#include "Housegeon_Game_Base.h"

#include "Dungeon_Generation/GS_DungeonGeneration.h"

void AHousegeon_Game_Base::StartPlay()
{
	Super::StartPlay();
	//Start the dungeon generation by creating the walls which will then be replaced once generation starts
	//(This function sets the size of DungeonGridInfo)
	Create_Starter_Walls();

	Create_Spawn_To_End();

	Create_Dead_End(4);

	//Create all the a to b paths that were put in an array from the created b points in the functions above
	//Create_Path_From_Start_To_End();

	//Once finished with the logic of the dungeon grid, pass it off to the game state for the information to
	//be global to all classes for movement logic as well as displaying info in world
	Dungeon_Logic_Finished();
}

void AHousegeon_Game_Base::Create_Starter_Walls()
{
	DungeonGridInfo.SetNum(Grid_X_Size);

	for (int i = 0; i < DungeonGridInfo.Num(); i++)
	{
		DungeonGridInfo[i].SetNum(Grid_Y_Size);
	}
}

void AHousegeon_Game_Base::Create_Spawn_To_End()
{
	//Get the absolute middle of the grid to spawn the player in
	int Start_X = DungeonGridInfo.Num() / 2;
	int Start_Y = DungeonGridInfo[0].Num() / 2;

	//Create a 3x3 square around the spawn point, just to make the spawn look good
	for (int x = Start_X - 1; x < Start_X + 2; x++)
	{
		for (int y = Start_Y - 1; y < Start_Y + 2; y++)
		{
			if (DungeonGridInfo.IsValidIndex(x)) 
			{
				if (DungeonGridInfo[x].IsValidIndex(y)) 
				{
					DungeonGridInfo[x][y] = EDungeonGenerationType::Spawn;
				}
			}
		}
	}

	//Init the end point for the grid
	int EndX;
	int EndY;

	//Call a function to pick the random edges for the end points (Passed by reference)
	Set_Random_End_XY(EndX, EndY);

	if (DungeonGridInfo.IsValidIndex(EndX)) 
	{
		if (DungeonGridInfo[EndX].IsValidIndex(EndY)) 
		{
			//Create the indexed location to have an endpoint, the endpoint will be an actor that allows the player
			//To move to the next level when activated
			DungeonGridInfo[EndX][EndY] = EDungeonGenerationType::EndPoint;
		}
		else 
		{
			UE_LOG(LogTemp, Error, TEXT("THE Y IS OUT OF BOUNDS FOR END POINT"));
		}
	}
	else 
	{
		UE_LOG(LogTemp, Error, TEXT("THE X IS OUT OF BOUNDS FOR END POINT"));
	}
	
	//After registering the end point, push it to the end location array which will be integral to making
	//paths from a to b
	
	FEnd_Location_Data EndData;
	EndData.X = EndX;
	EndData.Y = EndY;

	MyEndLocations.Add(EndData);
	//DungeonGridInfo[Start_X][Start_Y] = EDungeonGenerationType::Spawn;
}

void AHousegeon_Game_Base::Create_Dead_End(int Amount)
{
	for (int i = 0; i < Amount; i++) 
	{
		//Create the random x and y locations
		int RandX = FMath::RandRange(0, DungeonGridInfo.Num() - 1);
		int RandY = FMath::RandRange(0, DungeonGridInfo[0].Num() - 1);

		//Set the max amount of tries to brute force a location to be spawned
		int MaxTries = 300;

		//Brute force a location by trying multiple times, break when successful
		for (int Tries = 0; Tries < MaxTries; Tries++) 
		{
			if (DungeonGridInfo[RandX][RandY] == EDungeonGenerationType::Wall) 
			{
				//Create the dead end which will just be a floor
				DungeonGridInfo[RandX][RandY] = EDungeonGenerationType::Floor;

				//Create the struct and add necessary variables
				FEnd_Location_Data LocationData;
				LocationData.X = RandX;
				LocationData.Y = RandY;

				//Add to the end locations to make path after every POI is created
				MyEndLocations.Add(LocationData);

				//Found successful place so break the for loop because don't have to try any more
				break;
			}
			if (Tries == MaxTries - 1) 
			{
				UE_LOG(LogTemp, Error, TEXT("Could Not Find Place for Dead End POI"));
			}
		}
	}
}

void AHousegeon_Game_Base::Set_Random_End_XY(int& EndX, int& EndY)
{
	//Choose between the max or min for the axes
	int RandX = FMath::RandRange(0, 1);
	int RandY = FMath::RandRange(0, 1);

	//If 0, I want the coord to be 0, else = max size of grid
	if (RandX == 0)
	{
		EndX = 0;
	}
	else
	{
		EndX = DungeonGridInfo.Num() - 1;
	}

	if (RandY == 0)
	{
		EndY = 0;
	}
	else
	{
		EndY = DungeonGridInfo[0].Num() - 1;
	}

	UE_LOG(LogTemp, Display, TEXT("X end point = %d"), EndX);
	UE_LOG(LogTemp, Display, TEXT("Y end point = %d"), EndY);
}

void AHousegeon_Game_Base::Create_Path_From_Start_To_End()
{
	int Player_Start_Row;
	int Player_Start_Column;

	EPath_Moved PathMoved; //Path moved from center

	//I go through all the end locations and then create paths for all of them
	for (int i = 0; i < MyEndLocations.Num(); i++) 
	{
		Player_Start_Row = Grid_X_Size / 2;
		Player_Start_Column = Grid_Y_Size / 2;

		//Player_Start_Row = Grid_Y_Size / 2;
		//Player_Start_Column = Grid_X_Size / 2;

		int Player_Start_Random = FMath::RandRange(0, 3); //Random start location
		//Player_Start_Random = 1; //Debugging Number Thing

		//Pick randomly from the playerstart a start location. Goes clockwise starting from up
		switch (Player_Start_Random)
		{
			//Up from player start
		case 0:
			//Go 2 up for the index. Minus because that's the way it is mate
			UE_LOG(LogTemp, Display, TEXT("Randomlly selected UP to start path algorithm"));
			Player_Start_Column -= 2;
			PathMoved = EPath_Moved::UP;
			break;

			//Right from player start
		case 1:
			//Go 2 right
			UE_LOG(LogTemp, Display, TEXT("Randomlly selected RIGHT to start path algorithm"));
			Player_Start_Row += 2;
			PathMoved = EPath_Moved::RIGHT;
			break;

			//Down from player start
		case 2:
			UE_LOG(LogTemp, Display, TEXT("Randomlly selected DOWN to start path algorithm"));
			Player_Start_Column += 2;
			PathMoved = EPath_Moved::DOWN;
			break;

			//Left from player start
		case 3:
			UE_LOG(LogTemp, Display, TEXT("Randomlly selected LEFT to start path algorithm"));
			Player_Start_Row -= 2;
			PathMoved = EPath_Moved::LEFT;
			break;

		default:
			UE_LOG(LogTemp, Error, TEXT("The random start for player start in path algorithm failed"));
		}

		//Create the first floor to the a to b path algorithm as the function below will build the floor from this
		DungeonGridInfo[Player_Start_Row][Player_Start_Column] = EDungeonGenerationType::Floor;
	}
}

void AHousegeon_Game_Base::Dungeon_Logic_Finished()
{
	AGS_DungeonGeneration* DungeonGameState = GetWorld()->GetGameState<AGS_DungeonGeneration>();

	if (DungeonGameState) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Found the dungeon generation game state!"));

		DungeonGameState->Set_Dungeon_Grid_Info(DungeonGridInfo);
	}
	else 
	{
		UE_LOG(LogTemp, Error, TEXT("NOT Found the dungeon generation game state!"));
	}
}
