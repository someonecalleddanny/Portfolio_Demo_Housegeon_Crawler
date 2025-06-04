// Fill out your copyright notice in the Description page of Project Settings.


#include "Housegeon_Game_Base.h"

#include "Dungeon_Generation/GS_DungeonGeneration.h"

void AHousegeon_Game_Base::StartPlay()
{
	Super::StartPlay();
	
	//On start play, have all the default generation, once in play, can debug to what I want
	Generate_Dungeon(ChoiceOfTraversal, WhichAxisFirst, WherePathSpawns);
}

void AHousegeon_Game_Base::Generate_Dungeon(EPathTraversalType TraversalType, EAxisTraversalChoice AxisType, FPathSpawnAreaChoice PathSpawn)
{
	//Before starting the generation, check what options were made to generate the dungeon, All global UPROPERTY attributes
	ChoiceOfTraversal = TraversalType;
	WhichAxisFirst = AxisType;
	WherePathSpawns = PathSpawn;

	//Restart all arrays (Debugging for now but might make feature, to re-generate the dungeon)
	DungeonGridInfo = TArray<TArray<EDungeonGenerationType>>();
	MyEndLocations = TArray<FEnd_Location_Data>();

	//Start the dungeon generation by creating the walls which will then be replaced once generation starts
	//(This function sets the size of DungeonGridInfo)
	Create_Starter_Walls();

	//Create the Spawn location (center) + a 3x3 area around it as well as marking the endpoint randomly
	Create_Spawn_To_End();

	Create_Dead_End(Amount_Of_Dead_Ends);

	Create1x1_ChestPOI(Amount_Of_Chest_POIs);

	//Create all the a to b paths that were put in an array from the created b points in the functions above
	Create_Path_From_Start_To_End();

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

	//Create a 3x3 walkable square around the spawn point, just to make the spawn look good for traversal
	for (int x = Start_X - 1; x < Start_X + 2; x++)
	{
		for (int y = Start_Y - 1; y < Start_Y + 2; y++)
		{
			if (DungeonGridInfo.IsValidIndex(x)) 
			{
				if (DungeonGridInfo[x].IsValidIndex(y)) 
				{
					DungeonGridInfo[x][y] = EDungeonGenerationType::Floor;
				}
			}
		}
	}

	//Then, after making the 3x3 floor, I want the center to be the spawn as the center spawn will
	//Have unique properties such as adding unique things to that location such as a compass for the map viewer
	DungeonGridInfo[Start_X][Start_Y] = EDungeonGenerationType::Spawn;

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
	//Check the grid is actually initted
	if (!(DungeonGridInfo.IsValidIndex(0))) return;
	if (!(DungeonGridInfo[0].IsValidIndex(0))) return;

	//Keep trying to spawn from the amount that was inputted
	for (int i = 0; i < Amount; i++) 
	{
		//Set the max amount of tries to brute force a location to be spawned
		int MaxTries = 300;

		//Brute force a location by trying multiple times, break when successful
		for (int Tries = 0; Tries < MaxTries; Tries++) 
		{
			//Create the random x and y locations and keep trying to get random locations if fail
			int RandX = FMath::RandRange(0, DungeonGridInfo.Num() - 1);
			int RandY = FMath::RandRange(0, DungeonGridInfo[0].Num() - 1);

			//Check if the random spawn is not within a certain bound of the spawn area
			if (!(RandX >= (Grid_X_Size / 2) - Spawn_Deadzone && RandX <= (Grid_X_Size / 2) + Spawn_Deadzone
				&& RandY >= (Grid_Y_Size / 2) - Spawn_Deadzone && RandY <= (Grid_Y_Size / 2) + Spawn_Deadzone))
			{
				//Check if wall, if POI or endpoint etc, I don't want to replace them
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
			}
			if (Tries == MaxTries - 1) 
			{
				UE_LOG(LogTemp, Error, TEXT("Could Not Find Place for Dead End POI"));
			}
		}
	}
}

void AHousegeon_Game_Base::Create1x1_ChestPOI(int Amount)
{
	//Just realized that I can create any nxn poi, but for now I want to localise it to one function as I don't have time
	//to optimse for more POI's + only going to be making a Chest POI for the next month.

	/*
		Future Danny, Are you making another POI? Make this function incorporate multiple POIs!!!
	*/

	//Check the grid is actually initted
	if (!(DungeonGridInfo.IsValidIndex(0))) return;
	if (!(DungeonGridInfo[0].IsValidIndex(0))) return;

	//Get the AABB collision bounds of the 3x3 spawn square + a gap to make sure no paths overlap with spawn
	int A_SpawnXSize = (Grid_X_Size / 2) - Spawn_Deadzone;
	int B_SpawnXSize = (Grid_X_Size / 2) + Spawn_Deadzone;
	int A_SpawnYSize = (Grid_Y_Size / 2) - Spawn_Deadzone;
	int B_SpawnYSize = (Grid_Y_Size / 2) + Spawn_Deadzone;

	//Keep trying to spawn from the amount that was inputted
	for (int i = 0; i < Amount; i++) 
	{
		//Set the max amount of tries to brute force a location to be spawned
		int MaxTries = 300;

		//Brute force a location by trying multiple times, break when successful
		for (int Tries = 0; Tries < MaxTries; Tries++) 
		{
			//Create the random x and y locations, keep trying new locations if fail
			int RandX = FMath::RandRange(0, DungeonGridInfo.Num() - 1);
			int RandY = FMath::RandRange(0, DungeonGridInfo[0].Num() - 1);

			//This is the walkable perimeter area around the center POI, this will be checked for collision when generating
			int A_XSize = RandX - 1;
			int B_XSize = RandX + 1;
			int A_YSize = RandY - 1;
			int B_YSize = RandY + 1;
			

			//if the walkable perimeters ever go outside the array, continue to the next try
			if (A_XSize < 0 || B_XSize > Grid_X_Size - 1) continue;
			if (A_YSize < 0 || B_YSize > Grid_Y_Size - 1) continue;

			//Then check if the walkable perimeter overlaps with spawn by doing an AABB collision bounds check
			bool bOverlapsSpawn =
				(A_XSize <= B_SpawnXSize && B_XSize >= A_SpawnXSize) &&
				(A_YSize <= B_SpawnYSize && B_YSize >= A_SpawnYSize);

			if (bOverlapsSpawn)
				continue; // Skip this location and try again

			bool CanSpawnPOI = true;
			//Now check if each of the cell locations overlap with any other important structures such as POIs, endpoints
			for (int XChecker = A_XSize; XChecker <= B_XSize; XChecker++)
			{
				for (int YChecker = A_YSize; YChecker <= B_YSize; YChecker++)
				{
					//Might look ugly, but check if any of the cell locations are overlapping with important structures
					//Need to go through for loop here because need to verify each cell
					//If wall, that is a free space, if floor, that is most likely a dead end space that can be overwritten
					//If the cell type is neither a wall AND not a floor, set the bool to false to try POI spawn again
					if (DungeonGridInfo[XChecker][YChecker] != EDungeonGenerationType::Wall
						&& DungeonGridInfo[XChecker][YChecker] != EDungeonGenerationType::Floor)
					{
						//Once once POI cell is corrupted, don't need to check other cells so break this for loop
						CanSpawnPOI = false;
						break;
					}
						
				}
				//Once once POI cell is corrupted, don't need to check other cells so break this for loop
				if (!CanSpawnPOI) break;
			}

			//If after check, the POI did overlap with important structures, try again
			if (!CanSpawnPOI) continue;

			//After all checks, I can now safely place the POI in the desired location with a walkable perimeter
			//Now check if each of the cell locations overlap with any other important structures such as POIs, endpoints
			for (int x = A_XSize; x <= B_XSize; x++)
			{
				for (int y = A_YSize; y <= B_YSize; y++)
				{
					(x == RandX && y == RandY)
						? DungeonGridInfo[x][y] = EDungeonGenerationType::ChestPOI
						: DungeonGridInfo[x][y] = EDungeonGenerationType::Floor;
				}
			}

			//Once finished with creating the POI break the for loop, because the try has been successful

			//Create the struct and add necessary variables
			FEnd_Location_Data LocationData;
			LocationData.X = RandX;
			LocationData.Y = RandY;

			//Add to the end locations to make path after every POI is created
			MyEndLocations.Add(LocationData);

			break;
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
	//Forward declare the start x and y's, will be changed for each new end location iterated through
	int Player_Start_Row;
	int Player_Start_Column;

	//Step 1: Choose a random spawn location from the spawn center which will then be used to move from a to b

	//I go through all the end locations and then create paths for all of them
	for (int i = 0; i < MyEndLocations.Num(); i++) 
	{
		//Start from the center of the grid
		Player_Start_Row = Grid_X_Size / 2;
		Player_Start_Column = Grid_Y_Size / 2;

		//--------------------------------------------------------------------------------------
		//
		//The important variables that generate a lot of the stuff below, Put here for ease of debugging

		//forward declare the variables
		//int Player_Start_Random = 0;
		bool bRowTraversalFirst = true;
		bool bLineTraversal = true;

		//Randomly choose the paths from the struct of bools called WherePathSpawns, these are a UPROPERTY
		//if they are in anyway invalid, the default option is to randomly choose one of all possible spawn locations
		EPath_Moved PathMoved = Get_Random_Spawn_Point();

		//Choose the random start from up,down,left,right from the center spawn square
		//Player_Start_Random = FMath::RandRange(0, 3); //Random start location
		
		//Choose the type of axis traversal, set when the dungeon is first called to be generated
		if (WhichAxisFirst == EAxisTraversalChoice::RowFirst) 
		{
			bRowTraversalFirst = true;
		}
		else if (WhichAxisFirst == EAxisTraversalChoice::ColumnFirst) 
		{
			bRowTraversalFirst = false;
		}
		else //if none of the other options were selected, just select a random type of traversal
		{
			//Randomly pick if being Row Traversal, set as a variable for help whilst debugging the patterns
			bRowTraversalFirst = FMath::RandBool();
		}
		
		//For now I created 2 different types of traversals but if I make more in the future, I can use the
		// EPathTraversalType ENUM to randomly pick others etc
		if (ChoiceOfTraversal == EPathTraversalType::LineBased) 
		{
			bLineTraversal = true;
		}
		else if (ChoiceOfTraversal == EPathTraversalType::StairBased)
		{
			bLineTraversal = false;
		}
		else //if none of the traversal types were selected, choose random
		{
			//Randomly pick if the traversal type will be a line or not
			bLineTraversal = FMath::RandBool();
		}
		

		//Debugging
		//bRowTraversalFirst = true;
		//bLineTraversal = false;
		//PathMoved = EPath_Moved::RIGHT;

		//
		//------------------------------------------------------------------------------------

		//Pick randomly from the playerstart a start location. Goes clockwise starting from up
		switch (PathMoved)
		{
			//Up from player start
		case EPath_Moved::UP:
			//Go 2 up for the index. Minus because that's the way it is mate
			UE_LOG(LogTemp, Display, TEXT("Randomlly selected UP to start path algorithm"));
			Player_Start_Column -= 2;
			break;

			//Right from player start
		case EPath_Moved::RIGHT:
			//Go 2 right
			UE_LOG(LogTemp, Display, TEXT("Randomlly selected RIGHT to start path algorithm"));
			Player_Start_Row += 2;
			break;

			//Down from player start
		case EPath_Moved::DOWN:
			UE_LOG(LogTemp, Display, TEXT("Randomlly selected DOWN to start path algorithm"));
			Player_Start_Column += 2;
			break;

			//Left from player start
		case EPath_Moved::LEFT:
			UE_LOG(LogTemp, Display, TEXT("Randomlly selected LEFT to start path algorithm"));
			Player_Start_Row -= 2;
			break;

		default:
			UE_LOG(LogTemp, Error, TEXT("The random start for player start in path algorithm failed"));
		}

		//Create the first floor to the a to b path algorithm as the function below will build the floor from this
		DungeonGridInfo[Player_Start_Row][Player_Start_Column] = EDungeonGenerationType::Floor;

		UE_LOG(LogTemp, Warning, TEXT("End X = %d"), MyEndLocations[i].X);
		UE_LOG(LogTemp, Warning, TEXT("End Y = %d"), MyEndLocations[i].Y);

		//Step 2: Now, create the paths from a to b with the path algorithms I created

		//check if row traversal is picked (going x and then y to desired coord)
		if (bRowTraversalFirst)
		{
			UE_LOG(LogTemp, Display, TEXT("Picked Row Traversal First For Path Pattern!"));

			Pick_Random_Row_Traversal(PathMoved, bLineTraversal, Player_Start_Row, Player_Start_Column,
				MyEndLocations[i].X, MyEndLocations[i].Y);
		}
		else 
		{
			Pick_Random_Column_Traversal(PathMoved, bLineTraversal, Player_Start_Row, Player_Start_Column,
				MyEndLocations[i].X, MyEndLocations[i].Y);
		}
	}
}

EPath_Moved AHousegeon_Game_Base::Get_Random_Spawn_Point()
{
	TArray<EPath_Moved> RandomSpawnContainer;

	//Check through all the global UPROPERTY struct bools from WherePathSpawns, if any are true, add to the random container
	//for one to be chosen
	//if all are false (possible logical error) make them all possible to be randomly chosen

	if (WherePathSpawns.bSpawnUp)
	{
		RandomSpawnContainer.Add(EPath_Moved::UP);
	}
	if (WherePathSpawns.bSpawnDown)
	{
		RandomSpawnContainer.Add(EPath_Moved::DOWN);
	}
	if (WherePathSpawns.bSpawnLeft)
	{
		RandomSpawnContainer.Add(EPath_Moved::LEFT);
	}
	if (WherePathSpawns.bSpawnRight)
	{
		RandomSpawnContainer.Add(EPath_Moved::RIGHT);
	}
	//if all the bools are false, this means that an error occured but I am changing all (only) 0's and 
	// (only) 1's to mean doing random paths on all spawn points
	if (!(WherePathSpawns.bSpawnUp) && !(WherePathSpawns.bSpawnDown)
		&& !(WherePathSpawns.bSpawnLeft) && !(WherePathSpawns.bSpawnRight))
	{
		//Add all of the possible points to the random container
		RandomSpawnContainer.Add(EPath_Moved::UP);
		RandomSpawnContainer.Add(EPath_Moved::DOWN);
		RandomSpawnContainer.Add(EPath_Moved::LEFT);
		RandomSpawnContainer.Add(EPath_Moved::RIGHT);

		//then update the uproperties to all be true for any class that needs to know they are true
		WherePathSpawns.bSpawnUp = true;
		WherePathSpawns.bSpawnDown = true;
		WherePathSpawns.bSpawnLeft = true;
		WherePathSpawns.bSpawnRight = true;
	}

	return RandomSpawnContainer[FMath::RandRange(0, RandomSpawnContainer.Num() - 1)];
}

void AHousegeon_Game_Base::Pick_Random_Row_Traversal(EPath_Moved PathMoved, bool bLineTraversal,
	int Player_Start_Row, int Player_Start_Column, int EndX, int EndY)
{

	//Check where the spawn was for the path to start as the algorithms differ slightly as they have to move
	//Around the 3x3 spawn to make the paths more structured and beautiful
	switch (PathMoved)
	{
		//Spawned left
	case EPath_Moved::LEFT:

		if (bLineTraversal)
		{
			//Line traversal, Go until start x == to end x, then go until start y == end y
			SpawnedLeftRight_RowFirst_LineTraversal(true, Player_Start_Row, Player_Start_Column, EndX, EndY);
		}
		else
		{
			//Stair traversal, randomly choose between 2 and 3 on x and y to do a stair like pattern to coord
			SpawnedLeft_RowFirst_StairTraversal(Player_Start_Row, Player_Start_Column,
				EndX, EndY, FMath::RandRange(2, 3), FMath::RandRange(2, 3));
		}

		break;

		//Spawned Right
	case EPath_Moved::RIGHT:

		if (bLineTraversal)
		{
			//Line traversal, Go until start x == to end x, then go until start y == end y
			SpawnedLeftRight_RowFirst_LineTraversal(false, Player_Start_Row, Player_Start_Column, EndX, EndY);
		}
		else
		{
			//Stair traversal, randomly choose between 2 and 3 on x and y to do a stair like pattern to coord
			SpawnedRight_RowFirst_StairTraversal(Player_Start_Row, Player_Start_Column,
				EndX, EndY, FMath::RandRange(2, 3), FMath::RandRange(2, 3));
		}

		break;

		//Spawned Up
	case EPath_Moved::UP:

		if (bLineTraversal)
		{
			//Line traversal, Go until start x == to end x, then go until start y == end y
			SpawnedUpDown_RowFirst_LineTraversal(true, Player_Start_Row, Player_Start_Column, EndX, EndY);
		}
		else
		{
			//Stair traversal, randomly choose between 2 and 3 on x and y to do a stair like pattern to coord
			SpawnedUpDown_RowFirst_StairTraversal(true, Player_Start_Row, Player_Start_Column,
				EndX, EndY, FMath::RandRange(2, 3), FMath::RandRange(2, 3));
		}

		break;

		//Spawned Down
	case EPath_Moved::DOWN:

		if (bLineTraversal)
		{
			//Line traversal, Go until start x == to end x, then go until start y == end y
			SpawnedUpDown_RowFirst_LineTraversal(false, Player_Start_Row, Player_Start_Column, EndX, EndY);
		}
		else
		{
			//Stair traversal, randomly choose between 2 and 3 on x and y to do a stair like pattern to coord
			SpawnedUpDown_RowFirst_StairTraversal(false, Player_Start_Row, Player_Start_Column,
				EndX, EndY, FMath::RandRange(2, 3), FMath::RandRange(2, 3));
		}

		break;
	}
}

void AHousegeon_Game_Base::Pick_Random_Column_Traversal(EPath_Moved PathMoved, bool bLineTraversal, int Player_Start_Row, int Player_Start_Column, int EndX, int EndY)
{
	UE_LOG(LogTemp, Display, TEXT("Picked Column Traversal First For Path Pattern!"));

	//Check where the spawn was for the path to start as the algorithms differ slightly as they have to move
	//Around the 3x3 spawn to make the paths more structured and beautiful
	switch (PathMoved)
	{
		//Spawned left
	case EPath_Moved::LEFT:

		if (bLineTraversal)
		{
			//Do column first line traversal algorithm, input true at start because starting from left
			SpawnedLeftRight_ColumnFirst_LineTraversal(true, Player_Start_Row, Player_Start_Column,
				EndX, EndY);
		}
		else
		{
			//Do column based stair traversal, set the bool to true because the path moved is left from spawn
			SpawnedLeftRight_ColumnFirst_StairTraversal(true, Player_Start_Row, Player_Start_Column,
				EndX, EndY, FMath::RandRange(2, 3), FMath::RandRange(2, 3));

		}

		break;

		//Spawned Right
	case EPath_Moved::RIGHT:
		if (bLineTraversal)
		{
			//Do column first line traversal algorithm input false at start because starting from right
			SpawnedLeftRight_ColumnFirst_LineTraversal(false, Player_Start_Row, Player_Start_Column,
				EndX, EndY);
		}
		else
		{
			//Do column based stair traversal, set the bool to false because the path moved is right from spawn
			SpawnedLeftRight_ColumnFirst_StairTraversal(false, Player_Start_Row, Player_Start_Column,
				EndX, EndY, FMath::RandRange(2, 3), FMath::RandRange(2, 3));
		}
		break;

		//Spawned Up
	case EPath_Moved::UP:
		if (bLineTraversal)
		{
			SpawnedUpDown_ColumnFirst_LineTraversal(true, Player_Start_Row, Player_Start_Column,
				EndX, EndY);
		}
		else
		{
			SpawnedUp_ColumnFirst_StairTraversal(Player_Start_Row, Player_Start_Column,
				EndX, EndY, FMath::RandRange(2, 3), FMath::RandRange(2, 3));

		}
		break;

		//Spawned Down
	case EPath_Moved::DOWN:
		if (bLineTraversal)
		{
			SpawnedUpDown_ColumnFirst_LineTraversal(false, Player_Start_Row, Player_Start_Column,
				EndX, EndY);
		}
		else
		{
			SpawnedDown_ColumnFirst_StairTraversal(Player_Start_Row, Player_Start_Column,
				EndX, EndY, FMath::RandRange(2, 3), FMath::RandRange(2, 3));
		}
		break;
	}
}

void AHousegeon_Game_Base::SpawnedLeftRight_RowFirst_LineTraversal(bool bStartedFromLeft, int StartX, int StartY, int EndX, int EndY)
{
	// Validate bounds, if not, return which stops all logic from below from happening
	if (!DungeonGridInfo.IsValidIndex(StartX) || !DungeonGridInfo.IsValidIndex(EndX)) return;
	if (!DungeonGridInfo[StartX].IsValidIndex(StartY) || !DungeonGridInfo[EndX].IsValidIndex(EndY)) return;

	//First check whether the spawn is from left or right because this is important to knowing if the 3x3 spawn is behind
	//...The path algorithm (I don't want the path algorithm to go through spawn but around it)
	if (bStartedFromLeft) 
	{
		//I go left one more time because if I am going around the spawn, I want a wall's length around the spawn
		//Whilst, looping around it
		GO_LEFT(StartX, StartY);

		//if the ax is less than bx, that means that you have to go right, which is where the 3x3 spawn is, need to go around
		if (StartX < EndX)
		{
			//This is a bit different to the others, I don't want to go back through spawn
			//This means that I have to go around the spawn by going up or down first and then doing Row traversal
			//Choose randomly to go up or down because it does not need to be accurate as creating weird confusing
			//Paths is the point
			Go_Around_Spawn_RowBased_LeftRight(StartX, StartY, EndX, EndY);
		}
	}
	else //started on the right
	{
		//I go right one more time because if I am going around the spawn, I want a wall's length around the spawn
		//Whilst, looping around it
		GO_RIGHT(StartX, StartY);

		//if StartX is bigger means that the endx is to the left, since you spawned to the right that is behind
		//Therefore, you must go around the 3x3 spawn
		if (StartX > EndX)
		{	
			//Same explanation as above when used the go around spawn
			Go_Around_Spawn_RowBased_LeftRight(StartX, StartY, EndX, EndY);
		}
	}

	//Once the check for moving past the 3x3 spawn is over, do the row first line path algorithm
	DO_RowFirst_LineAlgorithm(StartX, StartY, EndX, EndY);
}

void AHousegeon_Game_Base::SpawnedUpDown_RowFirst_LineTraversal(bool bStartedFromUp, int StartX, int StartY, int EndX, int EndY)
{
	//This function is more simple than the spawned left/right because there is more space for the paths to
	//travel when the x axis is not being blocked by the center spawn

	// Validate bounds, if not, return which stops all logic from below from happening
	if (!DungeonGridInfo.IsValidIndex(StartX) || !DungeonGridInfo.IsValidIndex(EndX)) return;
	if (!DungeonGridInfo[StartX].IsValidIndex(StartY) || !DungeonGridInfo[EndX].IsValidIndex(EndY)) return;

	//Forward declare variables
	bool bDoGoAround = false;

	//First off, move one more down or up, because going horizontal first, meaning I don't want the 3x3 spawn to look ugly
	//When going left to right
	if (bStartedFromUp) 
	{
		GO_UP(StartX, StartY);

		//To go around, check if the end x is within the 3x3 row span (+ deadzone to create a gap between path and 3x3 spawn)
		//Then check if it is behind the path, don't want path going in on itself
		bDoGoAround = (EndX >= (Grid_X_Size / 2) - Spawn_Deadzone - 1 && EndX <= (Grid_X_Size / 2) + Spawn_Deadzone + 1
			&& EndY > StartY + 1); //On this line, check if the endy is behind where the path needs to go
	}
	else //means that you started from the down spawn
	{
		//Go down one more time to make a nice gap
		GO_DOWN(StartX, StartY);

		//To go around, check if the end x is within the 3x3 row span (+ deadzone to create a gap between path and 3x3 spawn)
		//Then check if it is behind the path, don't want path going in on itself
		bDoGoAround = (EndX >= (Grid_X_Size / 2) - Spawn_Deadzone - 1 && EndX <= (Grid_X_Size / 2) + Spawn_Deadzone + 1
			&& EndY < StartY - 1); //On this line, check if the endy is behind where the path needs to go
	}

	//If the checks above find that the path needs to go around the 3x3 spawn then do the helper function below
	if (bDoGoAround)
	{
		//An updated version to go around spawn when you want to to a horizontal first line
		Go_Around_Spawn_Vertical_Rated_Version(bStartedFromUp, StartX, StartY);
	}

	//Once the check for moving past the 3x3 spawn is over, do the row first line path algorithm
	DO_RowFirst_LineAlgorithm(StartX, StartY, EndX, EndY);
}

void AHousegeon_Game_Base::SpawnedLeft_RowFirst_StairTraversal(int StartX, int StartY, int EndX, int EndY, int X_Increment, int Y_Increment)
{
	// Validate bounds, if not, return which stops all logic from below from happening
	if (!DungeonGridInfo.IsValidIndex(StartX) || !DungeonGridInfo.IsValidIndex(EndX)) return;
	if (!DungeonGridInfo[StartX].IsValidIndex(StartY) || !DungeonGridInfo[EndX].IsValidIndex(EndY)) return;

	//Go left one more time to leave a nice gap between spawn
	GO_LEFT(StartX, StartY);

	//if the endpoint is behind the start of the path as well as the column bounds of the 3x3 spawn,
	//it means the endpoint is within something I like to call the 3x3 spawn shadow. Essentially, the column span
	//of the spawn persists along the x axis (the x axis behind the start of spawn) and needs to be bypassed before
	//doing the stair algorithm
	if (EndX > StartX
		&& EndY > (Grid_Y_Size / 2) - Spawn_Deadzone - 1
		&& EndY < (Grid_Y_Size / 2) + Spawn_Deadzone + 1)
	{
		bool bGoUp = FMath::RandBool();

		for (int i = 0; i < Spawn_Deadzone + 1; i++) 
		{
			(bGoUp)? GO_UP(StartX, StartY) : GO_DOWN(StartX, StartY);
		}
		
		//Once reached top or bottom edge (with sizable padding) go to the opposite half to reach the spawn shadow
		while (StartX < (Grid_X_Size / 2) + Spawn_Deadzone)
		{
			GO_RIGHT(StartX, StartY);
		}
	}
	else if (EndX > StartX)
	{
		//Check whether the stair has to go to the top half or the bottom half of the grid
		bool bGoUp = (StartY > EndY);

		//Idea here is to go to the left half of the grid before going up/down
		while (StartX != (Grid_X_Size / 4))
		{
			GO_LEFT(StartX, StartY);
		}

		//Passing note ----------------------------------------------------------------------------
		//Go past the dead zone of the center 3x3 (A bit late but I now know this is safer than my while loops because
		//there is actually an escape condition but I'll hardcode the dungeon to be never smaller than 10 (basically
		//my checks for the deadzone assume that there is more than 5x5 space for random generation but having a 5x5
		//dungeon just makes the whole generation worthless, might as well make a manual level)
		//-------------------------------------------------------------------------------------------------

		//Go to the upmost or downmost edge of the 3x3 spawn with a gap to make paths not overlap with spawn
		for (int i = 0; i < Spawn_Deadzone + 1; i++)
		{
			//simple if else, if need to go up, then go up, else is true when needing to go down
			(bGoUp)
				? GO_UP(StartX, StartY)
				: GO_DOWN(StartX, StartY);
		}
	}

	//Once done with every check possible to bypass the 3x3 spawn, do the stair algorithm
	//Key thing to note that, if the endpoint is on the left half of the grid, don't need to bypass anything so this function
	//is called right away
	DO_RowFirst_StairAlgorithm(StartX, StartY, EndX, EndY, X_Increment, Y_Increment);
}

void AHousegeon_Game_Base::SpawnedRight_RowFirst_StairTraversal(int StartX, int StartY, int EndX, int EndY, int X_Increment, int Y_Increment)
{
	// Validate bounds, if not, return which stops all logic from below from happening
	if (!DungeonGridInfo.IsValidIndex(StartX) || !DungeonGridInfo.IsValidIndex(EndX)) return;
	if (!DungeonGridInfo[StartX].IsValidIndex(StartY) || !DungeonGridInfo[EndX].IsValidIndex(EndY)) return;

	//Go Right one more time to leave a nice gap between spawn
	GO_RIGHT(StartX, StartY);

	//if the endpoint is behind the start of the path as well as the column bounds of the 3x3 spawn,
	//it means the endpoint is within something I like to call the 3x3 spawn shadow. Essentially, the column span
	//of the spawn persists along the x axis (the x axis behind the start of spawn) and needs to be bypassed before
	//doing the stair algorithm
	if (EndX < StartX
		&& EndY > (Grid_Y_Size / 2) - Spawn_Deadzone - 1
		&& EndY < (Grid_Y_Size / 2) + Spawn_Deadzone + 1)
	{
		//randomly choose from going up or down
		bool bGoUp = FMath::RandBool();

		//keep going up or down until reaching the top or bottom edge of the 3x3 spawn
		for (int i = 0; i < Spawn_Deadzone + 1; i++) 
		{
			(bGoUp)? GO_UP(StartX, StartY) : GO_DOWN(StartX, StartY);
		}

		//So keep going left until you reach the outmost left edge of the 3x3 spawn
		while (StartX != (Grid_X_Size / 2) - Spawn_Deadzone)
		{
			GO_LEFT(StartX, StartY);
		}
	}
	else if (EndX < StartX) 
	{
		//Check whether the stair has to go to the top half or the bottom half of the grid
		bool bGoUp = (StartY > EndY);

		//Idea here is to go to the right 3/4 of the grid, and the stair traversel will eventually go up/down
		// 3x3 spawn Whilst going right
		while (StartX != (Grid_X_Size * 3) / 4)
		{
			GO_RIGHT(StartX, StartY);
		}

		//Go to the upmost or downmost edge of the 3x3 spawn with a gap to make paths not overlap with spawn
		for (int i = 0; i < Spawn_Deadzone + 1; i++)
		{
			//simple if else, if need to go up, then go up, else is true when needing to go down
			(bGoUp)
				? GO_UP(StartX, StartY)
				: GO_DOWN(StartX, StartY);
		}
	}

	//Once done with every check possible to bypass the 3x3 spawn, do the stair algorithm
	//Key thing to note that, if the endpoint is on the right half of the grid, don't need to bypass anything so this function
	//is called right away
	DO_RowFirst_StairAlgorithm(StartX, StartY, EndX, EndY, X_Increment, Y_Increment);
}

void AHousegeon_Game_Base::SpawnedUpDown_RowFirst_StairTraversal(bool bStartedFromUp, int StartX, int StartY, int EndX, int EndY, int X_Increment, int Y_Increment)
{
	// Validate bounds, if not, return which stops all logic from below from happening
	if (!DungeonGridInfo.IsValidIndex(StartX) || !DungeonGridInfo.IsValidIndex(EndX)) return;
	if (!DungeonGridInfo[StartX].IsValidIndex(StartY) || !DungeonGridInfo[EndX].IsValidIndex(EndY)) return;

	//forward declare the variable
	bool bMoveInBetweenHalves = false;

	if (bStartedFromUp) 
	{
		//First move up one to leave a nice gap
		GO_UP(StartX, StartY);

		//Check if the endy is on the other half of the grid, the if statements below check which specific halves
		if (EndY > (StartY + 1)) //Do +1 because just moved up (Remember moving up is - so going back is +)
		{
			//first check if the end y is on the complete other half of the grid past the 3x3 spawn
			if (EndY > (Grid_Y_Size / 2) + Spawn_Deadzone 
				&& EndX > (Grid_X_Size / 2) - Spawn_Deadzone - 1
				&& EndX < (Grid_X_Size / 2) + Spawn_Deadzone + 1)
			{
				//function to go from one half of the grid to the other when the endY is on opposite side of 3x3 spawn
				//Passes StartY and StartX as reference so they will be changed when inputted
				Go_Around_Spawn_VerticalStair_Rated_Version(bStartedFromUp, StartX, StartY);
			}
			else //if not on other half, means that the path has to move to the inbetween spaces that the 3x3 spawn holds
			{
				bMoveInBetweenHalves = true;
			}
		}
	}
	else 
	{
		//First move down one to leave a nice gap
		GO_DOWN(StartX, StartY);

		//This means that the End Y is on the opposite side of the grid meaning you have to around 3x3 spawn
		if (EndY < (StartY - 1)) 
		{
			//Check if the End Y coord is on the opposite side of the 3x3 spawn
			//The y grid size + deadzone is used because if the y is within the 3x3 spawn shadow, do a check with
			//bMovedUpNeedMoveAround to see which center-ish side the path will have to go to
			if (EndY < (Grid_Y_Size / 2) - Spawn_Deadzone) //Deadzone is the area to which POIs can't spawn next to spawn
			{
				//function to go from one half of the grid to the other when the endY is on opposite side of 3x3 spawn
				//Passes StartY and StartX as reference so they will be changed when inputted
				Go_Around_Spawn_VerticalStair_Rated_Version(bStartedFromUp, StartX, StartY);
			}
			//if not on other side of grid but within the shadow of the row of 3x3 spawn, need to check later
			else
			{
				bMoveInBetweenHalves = true;
			}
		}
	}
	
	//This is the check regardless if spawned up or down, since the problem of the center 3x3 spawn shadow affects both...
	//...Move the path around the 3x3 spawn to the correct area to where the 3x3 row shadow inhabits
	if (bMoveInBetweenHalves)
	{
		//if the endx is within the left middle of grid but within shadow of 3x3 spawn
		if (EndX < (Grid_X_Size / 2) - Spawn_Deadzone) 
		{
			//keep going left until you reach the top left of 3x3 spawn + forcefield (Added -1 because I use != condition...
			//...for while loop)
			while (StartX != (Grid_X_Size / 2) - Spawn_Deadzone - 1) 
			{
				GO_LEFT(StartX, StartY);
			}
		}
		else if (EndX > (Grid_X_Size / 2) + Spawn_Deadzone) 
		{
			//keep going Right until you reach the top right of 3x3 spawn + forcefield (Added +1 because I use != condition...
			//...for while loop)
			while (StartX != (Grid_X_Size / 2) + Spawn_Deadzone + 1)
			{
				GO_RIGHT(StartX, StartY);
			}
		}
	}

	//Once done with every check possible to bypass the 3x3 spawn, do the stair algorithm
	DO_RowFirst_StairAlgorithm(StartX, StartY, EndX, EndY, X_Increment, Y_Increment);
}


void AHousegeon_Game_Base::DO_RowFirst_LineAlgorithm(int StartX, int StartY, int EndX, int EndY)
{
	while (StartX != EndX) 
	{
		//if the start x is bigger than the end x, means that you have to go left, inverse when startx is less
		(StartX > EndX) ? GO_LEFT(StartX, StartY) : GO_RIGHT(StartX, StartY);
	}

	while (StartY != EndY) 
	{
		//if start y is bigger, means it is below the end y and must go up, inverse for when start x is more
		(StartY > EndY) ? GO_UP(StartX, StartY) : GO_DOWN(StartX, StartY);
	}
}

void AHousegeon_Game_Base::DO_RowFirst_StairAlgorithm(int StartX, int StartY, int EndX, int EndY, int X_Increment, int Y_Increment)
{
	//So, While the start coords are not equal keep doing the stair loops until reaching destination
	while (StartX != EndX || StartY != EndY)
	{
		//So repeat the x traversal by the x_increment or until the startx is == to end x because don't want to keep looping
		for (int x = 0; x < X_Increment && StartX != EndX; x++)
		{
			//if end x is less than start x, means that you have to go left, else = inverse
			(EndX < StartX) ? GO_LEFT(StartX, StartY) : GO_RIGHT(StartX, StartY);
		}

		for (int y = 0; y < Y_Increment && StartY != EndY; y++)
		{
			//if end y lower, means that you have to go lower y index, going up, inverse for going down
			(EndY < StartY) ? GO_UP(StartX, StartY) : GO_DOWN(StartX, StartY);
		}
	}
}

void AHousegeon_Game_Base::DO_ColumnFirst_LineAlgorithm(int StartX, int StartY, int EndX, int EndY)
{
	//now just do the algorithm to move column and then row
	while (StartY != EndY)
	{
		//if start y is less that means it is above it and must go down, else, must go up
		(StartY < EndY) ? GO_DOWN(StartX, StartY) : GO_UP(StartX, StartY);
	}
	//Now do Row traversal
	while (StartX != EndX)
	{
		//if start x is less that means it is to the left of end x and must go right, else, must go left
		(StartX < EndX) ? GO_RIGHT(StartX, StartY) : GO_LEFT(StartX, StartY);
	}
}

void AHousegeon_Game_Base::DO_ColumnFirst_StairAlgorithm(int StartX, int StartY, int EndX, int EndY, int X_Increment, int Y_Increment)
{
	//Very similar to row first stair algorithm, with the for loops being switched, could merge into one function
	//but I think this increases readability of the code at the cost of increased line space

	//So, While the start coords are not equal keep doing the stair loops until reaching destination
	while (StartX != EndX || StartY != EndY)
	{
		for (int y = 0; y < Y_Increment && StartY != EndY; y++)
		{
			//if end y lower, means that you have to go lower y index, going up, inverse for going down
			(EndY < StartY) ? GO_UP(StartX, StartY) : GO_DOWN(StartX, StartY);
		}

		//So repeat the x traversal by the x_increment or until the startx is == to end x because don't want to keep looping
		for (int x = 0; x < X_Increment && StartX != EndX; x++)
		{
			//if end x is less than start x, means that you have to go left, else = inverse
			(EndX < StartX) ? GO_LEFT(StartX, StartY) : GO_RIGHT(StartX, StartY);
		}
	}
}

void AHousegeon_Game_Base::SpawnedLeftRight_ColumnFirst_LineTraversal(bool bStartedFromLeft, int StartX, int StartY, int EndX, int EndY)
{
	// Validate bounds, if not, return which stops all logic from below from happening
	if (!DungeonGridInfo.IsValidIndex(StartX) || !DungeonGridInfo.IsValidIndex(EndX)) return;
	if (!DungeonGridInfo[StartX].IsValidIndex(StartY) || !DungeonGridInfo[EndX].IsValidIndex(EndY)) return;

	//The spawned left and right functions are pretty similar so don't really need to be seperate functions

	if (bStartedFromLeft) 
	{
		//Go one more left to make nice path
		GO_LEFT(StartX, StartY);

		//if the end x is to the right of the path (behind) as well as being in the 3x3 spawn shadow bounds,
		//Go around the 3x3 Spawn
		if (EndX > StartX + 1 //+ 1 because just moved left
			&& EndY > (Grid_Y_Size / 2) - Spawn_Deadzone - 1
			&& EndY < (Grid_Y_Size / 2) + Spawn_Deadzone + 1)
		{
			Go_Around_Spawn_ColumnBased_LeftRight(bStartedFromLeft, StartX, StartY);
		}

	}
	else //This means you started on the right
	{
		//one more to right to make nice path
		GO_RIGHT(StartX, StartY);

		//if the end x is to the left of the path (behind) as well as being in the 3x3 spawn shadow bounds,
		//Go around the 3x3 Spawn
		if (EndX < StartX - 1 //- 1 because just moved right
			&& EndY > (Grid_Y_Size / 2) - Spawn_Deadzone - 1
			&& EndY < (Grid_Y_Size / 2) + Spawn_Deadzone + 1)
		{
			Go_Around_Spawn_ColumnBased_LeftRight(bStartedFromLeft, StartX, StartY);
		}
	}

	//Now do the column first algorithm with the 3x3 spawn out of the way
	DO_ColumnFirst_LineAlgorithm(StartX, StartY, EndX, EndY);
}

void AHousegeon_Game_Base::SpawnedUpDown_ColumnFirst_LineTraversal(bool bStartedFromUp, int StartX, int StartY, int EndX, int EndY)
{
	// Validate bounds, if not, return which stops all logic from below from happening
	if (!DungeonGridInfo.IsValidIndex(StartX) || !DungeonGridInfo.IsValidIndex(EndX)) return;
	if (!DungeonGridInfo[StartX].IsValidIndex(StartY) || !DungeonGridInfo[EndX].IsValidIndex(EndY)) return;

	if (bStartedFromUp) //Started from up
	{
		//Go up one more time to avoid overlapping with spawn
		GO_UP(StartX, StartY);

		//if the endy is more than the start y, that means it is behind, the 3x3 spawn is also behind, so have to go around
		if (EndY > StartY) 
		{
			Go_Around_Spawn_ColumnBased_UpDown(StartX, StartY, EndX, EndY);
		}
	}
	else //Started from down
	{
		//Go down one more to give a nice gap
		GO_DOWN(StartX, StartY);

		//if the endy is less than the start y, that means it is behind (Since end y is above by being less than start y)
		// the 3x3 spawn is also behind, so have to go around
		if (EndY < StartY)
		{
			Go_Around_Spawn_ColumnBased_UpDown(StartX, StartY, EndX, EndY);
		}
	}

	//Now do the column first algorithm with the 3x3 spawn out of the way
	DO_ColumnFirst_LineAlgorithm(StartX, StartY, EndX, EndY);
}

void AHousegeon_Game_Base::SpawnedUp_ColumnFirst_StairTraversal(int StartX, int StartY, int EndX, int EndY, int X_Increment, int Y_Increment)
{
	// Validate bounds, if not, return which stops all logic from below from happening
	if (!DungeonGridInfo.IsValidIndex(StartX) || !DungeonGridInfo.IsValidIndex(EndX)) return;
	if (!DungeonGridInfo[StartX].IsValidIndex(StartY) || !DungeonGridInfo[EndX].IsValidIndex(EndY)) return;

	//Go Up one more time to avoid overlap with 3x3 spawn
	GO_UP(StartX, StartY);

	//if endy is more, that means it's below/behind, also check if the 3x3 rowspan shadow is in bounds of the end x
	//Remember the shadow goes an infinite distance along the behind y axis
	if (EndY > StartY
		&& EndX > (Grid_X_Size / 2) - Spawn_Deadzone - 1
		&& EndX < (Grid_X_Size / 2) + Spawn_Deadzone + 1)
	{
		//Randomly go right or left because going to the center-ish point
		//If still confused, still will reach point regardless if going left or right since the path makes a perfect square
		//Choose to be random because it reduces repetition in the paths
		bool bGoRight = FMath::RandBool();

		//keep going left/right until reaching deadzone
		for (int i = 0; i < Spawn_Deadzone + 1; i++)
		{
			(bGoRight) ? GO_RIGHT(StartX, StartY) : GO_LEFT(StartX, StartY);
		}

		//So keep going down until you reach the downmost edge of the 3x3 spawn
		while (StartY != (Grid_Y_Size / 2) + Spawn_Deadzone)
		{
			GO_DOWN(StartX, StartY);
		}
	}
	else if (EndY > StartY)//Now just checking if behind
	{
		//Check whether the stair has to go to the top half or the bottom half of the grid
		bool bGoRight = (StartX < EndX);

		//idea here is to go to the top middle of the grid before bypassing spawn
		while (StartY != (Grid_Y_Size / 4))
		{
			GO_UP(StartX, StartY);
		}

		//Now go to the right/left - most edge of 3x3 spawn
		for (int i = 0; i < Spawn_Deadzone + 1; i++)
		{
			//simple if else, if need to go up, then go up, else is true when needing to go down
			(bGoRight)
				? GO_RIGHT(StartX, StartY)
				: GO_LEFT(StartX, StartY);
		}
	}

	//Once all of the bypassing functions are finished, now the column based stair algorithm can be used
	DO_ColumnFirst_StairAlgorithm(StartX, StartY, EndX, EndY, X_Increment, Y_Increment);
}

void AHousegeon_Game_Base::SpawnedDown_ColumnFirst_StairTraversal(int StartX, int StartY, int EndX, int EndY, int X_Increment, int Y_Increment)
{
	// Validate bounds, if not, return which stops all logic from below from happening
	if (!DungeonGridInfo.IsValidIndex(StartX) || !DungeonGridInfo.IsValidIndex(EndX)) return;
	if (!DungeonGridInfo[StartX].IsValidIndex(StartY) || !DungeonGridInfo[EndX].IsValidIndex(EndY)) return;

	//Go DOWN one more time to avoid overlap with 3x3 spawn
	GO_DOWN(StartX, StartY);

	//if endy is less, that means it's above/behind, also check if the 3x3 rowspan shadow is in bounds of the end x
	//Remember the shadow goes an infinite distance along the behind y axis
	if (EndY < StartY
		&& EndX > (Grid_X_Size / 2) - Spawn_Deadzone - 1
		&& EndX < (Grid_X_Size / 2) + Spawn_Deadzone + 1)
	{
		//Randomly go right or left because going to the center-ish point
		//If still confused, still will reach point regardless if going left or right since the path makes a perfect square
		//Choose to be random because it reduces repetition in the paths
		bool bGoRight = FMath::RandBool();

		//keep going left/right until reaching deadzone
		for (int i = 0; i < Spawn_Deadzone + 1; i++)
		{
			(bGoRight) ? GO_RIGHT(StartX, StartY) : GO_LEFT(StartX, StartY);
		}

		//So keep going UP until you reach the upmost edge of the 3x3 spawn
		while (StartY != (Grid_X_Size / 2) - Spawn_Deadzone)
		{
			GO_UP(StartX, StartY);
		}
	}
	else if (EndY < StartY)//Now just checking if behind
	{
		//Check whether the stair has to go to the top half or the bottom half of the grid
		bool bGoRight = (StartX < EndX);

		//idea here is to go to the bottom middle of the grid before bypassing spawn
		while (StartY != (Grid_Y_Size * 3) / 4)
		{
			GO_DOWN(StartX, StartY);
		}

		//Now go to the right/left - most edge of 3x3 spawn
		for (int i = 0; i < Spawn_Deadzone + 1; i++)
		{
			//simple if else, if need to go up, then go up, else is true when needing to go down
			(bGoRight)
				? GO_RIGHT(StartX, StartY)
				: GO_LEFT(StartX, StartY);
		}
	}

	//Once all of the bypassing functions are finished, now the column based stair algorithm can be used
	DO_ColumnFirst_StairAlgorithm(StartX, StartY, EndX, EndY, X_Increment, Y_Increment);
}

void AHousegeon_Game_Base::SpawnedLeftRight_ColumnFirst_StairTraversal(bool bStartedFromLeft, int StartX, int StartY, int EndX, int EndY, int X_Increment, int Y_Increment)
{
	// Validate bounds, if not, return which stops all logic from below from happening
	if (!DungeonGridInfo.IsValidIndex(StartX) || !DungeonGridInfo.IsValidIndex(EndX)) return;
	if (!DungeonGridInfo[StartX].IsValidIndex(StartY) || !DungeonGridInfo[EndX].IsValidIndex(EndY)) return;

	//forward declare the variable
	bool bMoveInBetweenHalves = false;

	if (bStartedFromLeft) //Started from left logic
	{
		//first move left one more time to make a nice gap
		GO_LEFT(StartX, StartY);

		//Check if the endx is on the other half of the grid, the if statements below check which specific halves
		if (EndX > (StartX + 1)) 
		{
			//first check if the endpoint is within bounds of the spawn 3x3 shadow which is essentially the whole size
			//of the spawn going across the x axis because this is the area that the path has to bypass to make nice path
			//(The Spawn shadow changes depending on where the path starts but in this case it is along the x axis for
			//spawning left/right)
			if (EndX > (Grid_X_Size / 2) + Spawn_Deadzone 
				&& EndY > (Grid_Y_Size / 2) - Spawn_Deadzone - 1
				&& EndY < (Grid_Y_Size / 2) + Spawn_Deadzone + 1)
			{
				Go_Around_Spawn_ColumnBasedStairs_LeftRight(bStartedFromLeft, StartX, StartY);
			}
			else //if not in spawn shadow, means that the path is behind but it is either on the top or bottom half of grid
			{
				bMoveInBetweenHalves = true;
			}
		}
	}
	else //Started from right logic
	{
		//First move right one more time to make a nice gap
		GO_RIGHT(StartX, StartY);

		//Check if the endx is on the other half of the grid, the if statements below check which specific halves
		if (EndX < (StartX + 1)) 
		{
			//first check if the endpoint is within bounds of the spawn 3x3 shadow which is essentially the whole size
			//of the spawn going across the x axis because this is the area that the path has to bypass to make nice path
			//(The Spawn shadow changes depending on where the path starts but in this case it is along the x axis for
			//spawning left/right)
			if (EndX < (Grid_X_Size / 2) - Spawn_Deadzone
				&& EndY >(Grid_Y_Size / 2) - Spawn_Deadzone - 1
				&& EndY < (Grid_Y_Size / 2) + Spawn_Deadzone + 1)
			{
				Go_Around_Spawn_ColumnBasedStairs_LeftRight(bStartedFromLeft, StartX, StartY);
			}
			else //if not in spawn shadow, means that the path is behind but it is either on the top or bottom half of grid
			{
				bMoveInBetweenHalves = true;
			}
		}
	}

	//This check happens regardless of where spawned, the idea is that there is a small row span from the 3x3 spawn
	//that is in between halves that makes the path look ugly 
	if (bMoveInBetweenHalves)
	{
		//if the end y is in the top middle of the grid
		if (EndY < (Grid_Y_Size / 2))
		{
			//keep going up until reaching the top of the 3x3 spawn
			while (StartY != (Grid_Y_Size / 2) - Spawn_Deadzone - 1)
			{
				GO_UP(StartX, StartY);
			}
		}
		else if (EndY > (Grid_Y_Size / 2)) //if in bottom middle of grid
		{
			//keep going down until reaching bottom edge of grid + deadzone to make a nice gap
			while (StartY != (Grid_Y_Size / 2) + Spawn_Deadzone + 1)
			{
				GO_DOWN(StartX, StartY);
			}
		}
	}

	//Once all of the bypassing functions are finished, now the column based stair algorithm can be used
	DO_ColumnFirst_StairAlgorithm(StartX, StartY, EndX, EndY, X_Increment, Y_Increment);
}

void AHousegeon_Game_Base::Go_Around_Spawn_RowBased_LeftRight(int& ChangedX, int& ChangedY, int EndX, int EndY)
{
	//Helper function to go around the 3x3 spawn to make the paths look better
	//Ironically this helper function is similar to "Go_Around_Spawn_ColumnBased_UpDown" (copy/pasted and changed)
	//Just think of rotating the whole grid and suddenly row first traversal looks like column based traversal
	//With that thought in mind, the copy/pasted logic is mirrored

	bool bEndpointInSpawnShadow = false;

	//Do a neat check if the end x is exactly equal to the spawn x, do this because the path can loop through the spawn
	//but it looks like it's coming from the up/down spawn making a cool loop pattern
	if (!(EndX == (Grid_X_Size / 2))) 
	{
		// First check if the end point is within the shadow of the entire 3x3 spawn
		//Do this because, do not want the column based traversel to go through the spawn if the endpoint is relatively in
		//the center (Creates an irregular spawn)
		if (EndX > (Grid_X_Size / 2) - Spawn_Deadzone - 1 && EndX < (Grid_X_Size / 2) + Spawn_Deadzone + 1)
		{
			bEndpointInSpawnShadow = true;
		}
	}

	if (bEndpointInSpawnShadow)
	{
		//Make a bool to easily visualise if I have to go up or not
		bool bGoUp = (ChangedY > EndY);

		//Looks confusing but I do it like this because I don't want 3 indents of if/elses + while loops
		//Basically, I create the edge y by checking which way I am going (Up/Down) if right, set edge y location
		//to the top edge of 3x3 spawn, else = the down edge of 3x3 spawn
		int EdgeY = (bGoUp)
			? (Grid_Y_Size / 2) - Spawn_Deadzone - 1
			: (Grid_Y_Size / 2) + Spawn_Deadzone + 1;

		//While you haven't gone to the 3x3 spawn edge keeping going up/down
		while (ChangedY != EdgeY)
		{
			//I do the ? below because I don't want to have 3 indents of ifs and whiles (IMO better readability)
			//This is as simple as an if else can get: if up? go up, else? go down
			(bGoUp) ? GO_UP(ChangedX, ChangedY) : GO_DOWN(ChangedX, ChangedY);
		}

		//Once the edge is reached I want to return because the below code assumes that the endpoint is not within the
		//3x3 spawn shadow area
		return;
	}

	//Choose randomly to either go to the upmost or downmost edge to bypass the 3x3 spawn area before doing row traversal
	if (FMath::RandBool())
	{
		// if the end x,y is kinda in the center of the grid, the path will go up or down to avoid the spawn area
		// and then continue the traversal in the row-based pattern

		while (ChangedY != (Grid_Y_Size / 2) - Spawn_Deadzone - 1) 
		{
			GO_UP(ChangedX, ChangedY);
		}
	}
	else
	{
		while (ChangedY != (Grid_Y_Size / 2) + Spawn_Deadzone + 1)
		{
			GO_DOWN(ChangedX, ChangedY);
		}
	}
}

void AHousegeon_Game_Base::Go_Around_Spawn_Vertical_Rated_Version(bool bStartedFromUp, int& ChangedX, int& ChangedY)
{
	//Randomly choose between going horizontlly right 3 times or left before going down
	//I started doing 1/4 and 3/4 x locations in other functions but I kinda want to keep this here as it eliminates
	//Repetition on bigger grids if there are different x move arounds going on
	if (FMath::RandBool())
	{
		for (int i = 0; i < 3; i++)
		{
			GO_RIGHT(ChangedX, ChangedY);
		}
	}
	else
	{
		for (int i = 0; i < 3; i++)
		{
			GO_LEFT(ChangedX, ChangedY);
		}
	}

	//If started from up, I will have to go around and then down to reach the pesky end location hidden
	//behind the 3x3 spawn sqaure
	if (bStartedFromUp) 
	{
		/*
			//old debug code
			//Then you have to go 7 down on y because: 3 = spawn, 2 = moving up, 2 = the gap to make beauty spawn
		for (int i = 0; i < 7; i++)
		{
			GO_DOWN(ChangedX, ChangedY);
		}
		*/

		//The y path has to go to the bottom edge of the 3x3 spawn + a big enough gap so the the path does not overlap
		while (ChangedY != (Grid_Y_Size / 2) + Spawn_Deadzone + 1)
		{
			GO_DOWN(ChangedX, ChangedY);
		}

	}
	else //So you started from down, do the inverse of above
	{
		/*
			//Old debug code
					//Then you have to go 7 UP on y because: 3 = spawn, 2 = moving down, 2 = the gap to make beauty spawn
		for (int i = 0; i < 7; i++)
		{
			GO_UP(ChangedX, ChangedY);
		}
		*/
		while (ChangedY != (Grid_Y_Size / 2) - Spawn_Deadzone - 1)
		{
			GO_UP(ChangedX, ChangedY);
		}
	}
}

void AHousegeon_Game_Base::Go_Around_Spawn_VerticalStair_Rated_Version(bool bStartedFromUp, int& ChangedX, int& ChangedY)
{
	//Since it is on the other half of the grid, I can make some cool patterns to reach it before doing stair pattern
	//So I kinda have the creative liberty here, and I decided to make 4 x spots to go around the 3x3 to make
	//generation more juicy

	// Step 1: Move left or right to offset from spawn center horizontally

	//Rand bool to check whether to go left or right
	if (FMath::RandBool())
	{
		//Pick between going to the right quarter of the grid or the the right edge of the 3x3 spawn square
		int TargetX = FMath::RandBool()
			? (Grid_X_Size * 3) / 4
			: (Grid_X_Size / 2) + Spawn_Deadzone + 1; //(Add deadzone to have a gap for the path)

		//keep going right until reaching the target x set above
		while (ChangedX != TargetX) 
		{
			GO_RIGHT(ChangedX, ChangedY);
		}

		/*
		//Another rand bool to check whether to slightly go over the edge of 3x3 or to 1/4 or 3/4 of grid
		if (FMath::RandBool())
		{
			//Idea here is to go to the right 3/4 of the grid, and the stair traversel will eventually go up/down
			// 3x3 spawn Whilst going right
			while (ChangedX != (Grid_X_Size * 3) / 4)
			{
				GO_RIGHT(ChangedX, ChangedY);
			}
		}
		else
		{
			while (ChangedX != (Grid_X_Size / 2) + Spawn_Deadzone + 1)
			{
				GO_RIGHT(ChangedX, ChangedY);
			}
		}
		*/
		
	}
	else //going to the left
	{
		//Pick between going to the left quarter of the grid or the left edge of the 3x3 spawn square
		int TargetX = FMath::RandBool()
			? Grid_X_Size / 4
			: (Grid_X_Size / 2) - Spawn_Deadzone - 1; //(Add deadzone to have a gap for the path)

		while (ChangedX != TargetX)
		{
			GO_LEFT(ChangedX, ChangedY);
		}
		/*
		//Another rand bool to check whether to slightly go over the edge of 3x3 or to 1/4 or 3/4 of grid
		if (FMath::RandBool())
		{
			//Idea here is to go to the 1/4 x of grid, and the stair traversel will eventually go up/down
			while (ChangedX != Grid_X_Size / 4)
			{
				GO_LEFT(ChangedX, ChangedY);
			}
		}
		else
		{
			//Go to the top left edge + forcefield of the 3x3 spawn area to make beautiful path
			while (ChangedX != (Grid_X_Size / 2) - Spawn_Deadzone - 1)
			{
				GO_LEFT(ChangedX, ChangedY);
			}
		}
		*/
	}

	// Step 2: Move vertically to reach other side of spawn

	if (bStartedFromUp) //do this if started from top of 3x3 spawn
	{
		//Once done with choosing the xcoord to bypass the 3x3 spawn, have to go down to reach other grid half
		while (ChangedY != (Grid_Y_Size / 2) + Spawn_Deadzone + 1)
		{
			GO_DOWN(ChangedX, ChangedY);
		}
	}
	else //Do this to go around when starting from the bottom
	{
		while (ChangedY != (Grid_Y_Size / 2) - Spawn_Deadzone - 1) 
		{
			GO_UP(ChangedX, ChangedY);
		}
	}
}

void AHousegeon_Game_Base::Go_Around_Spawn_ColumnBased_LeftRight(bool bStartedFromLeft, int& ChangedX, int& ChangedY)
{
	//Choose randomly to whether go up or down before traversing on row to the 3x3 shadow zone
	if (FMath::RandBool())
	{
		//While start y is not equal to the bottom of the 3x3 spawn keep going down until you reach it
		while (ChangedY != Grid_Y_Size / 2 + Spawn_Deadzone + 1)
		{
			GO_DOWN(ChangedX, ChangedY);
		}
	}
	else
	{
		//While start y is not equal to the top of the 3x3 spawn keep going up until you reach it
		while (ChangedY != Grid_Y_Size / 2 - Spawn_Deadzone - 1)
		{
			GO_UP(ChangedX, ChangedY);
		}
	}

	if (bStartedFromLeft)
	{
		//Keep going right until you reach the right edge of the 3x3 spawn square to finally bypass it
		while (ChangedX != (Grid_X_Size / 2) + Spawn_Deadzone + 1)
		{
			GO_RIGHT(ChangedX, ChangedY);
		}
	}
	else //This means you started on the right
	{
		//Keep going left until you reach the left edge of the 3x3 spawn square to finally bypass it
		while (ChangedX != (Grid_X_Size / 2) - Spawn_Deadzone - 1)
		{
			GO_LEFT(ChangedX, ChangedY);
		}
	}
}

void AHousegeon_Game_Base::Go_Around_Spawn_ColumnBased_UpDown(int& ChangedX, int& ChangedY, int EndX, int EndY)
{
	bool bEndpointInSpawnShadow = false;
	//Do a neat check if the end y is exactly equal to the spawn, do this because the path can loop through the spawn
	//but it looks like it's coming from the left/right spawn making a cool loop pattern
	if (!(EndY == (Grid_Y_Size / 2))) 
	{
		//First check if the end point is within the shadow of the entire 3x3 spawn
		//Do this because, do not want the column based traversel to go through the spawn if the endpoint is relatively in
		//the center (Creates an irregular spawn)
		if (EndY > (Grid_Y_Size / 2) - Spawn_Deadzone - 1 && EndY < (Grid_Y_Size / 2) + Spawn_Deadzone + 1)
		{
			bEndpointInSpawnShadow = true;
		}
	}

	if (bEndpointInSpawnShadow) 
	{
		//Make a bool to easily visualise if I have to go right or not
		bool bGoRight = (ChangedX < EndX);

		//Looks confusing but I do it like this because I don't want 3 indents of if/elses + while loops
		//Basically, I create the edge x by checking which way I am going (Left/Right) if right, set edge x location
		//to the right edge of 3x3 spawn, else = the leftmost edge of 3x3 spawn
		int EdgeX = (bGoRight)
			? (Grid_X_Size / 2) + Spawn_Deadzone + 1
			: (Grid_X_Size / 2) - Spawn_Deadzone - 1;

		//While you haven't gone to the 3x3 spawn edge keeping going left or right
		while (ChangedX != EdgeX) 
		{
			//I do the ? below because I don't want to have 3 indents of ifs and whiles (IMO better readability)
			//This is as simple as an if else can get: if right? go right, else? go left
			(bGoRight) ? GO_RIGHT(ChangedX, ChangedY) : GO_LEFT(ChangedX, ChangedY);
		}

		//Once the edge is reached I want to return because the below code assumes that the endpoint is not within the
		//3x3 spawn shadow area
		return;
	}
	//Choose randomly between going left or right
	if (FMath::RandBool()) 
	{
		//This is a while loop to go to the left most edge of the 3x3 spawn + gap for beauty
		while (ChangedX != (Grid_X_Size / 2) - Spawn_Deadzone - 1)
		{
			GO_LEFT(ChangedX, ChangedY);
		}
	}
	else 
	{
		//This is a while loop to go to the right most edge of the 3x3 spawn + gap for beauty
		while (ChangedX != (Grid_X_Size / 2) + Spawn_Deadzone + 1)
		{
			GO_RIGHT(ChangedX, ChangedY);
		}
	}
}

void AHousegeon_Game_Base::Go_Around_Spawn_ColumnBasedStairs_LeftRight(bool bStartedFromLeft, int& ChangedX, int& ChangedY)
{
	//Since it is on the other half of the grid, I can make some cool patterns to reach it before doing stair pattern
	//So I kinda have the creative liberty here, and I decided to make 4 y spots to go around the 3x3 to make
	//generation more juicy

	// Step 1: Choose randomly to go up or down with an added randomness. Can be used if spawned from left or right
	//because the path is not going from their relative behinds

	//Rand bool to check whether to go down or up
	if (FMath::RandBool()) //if true = going down
	{
		//Choose between the bottom quarter of the grid or the bottom edge of the spawn area
		int TargetY = FMath::RandBool()
			? (Grid_Y_Size * 3) / 4
			: (Grid_Y_Size / 2) + Spawn_Deadzone + 1;

		while (ChangedY != TargetY)
		{
			GO_DOWN(ChangedX, ChangedY);
		}

		/*
		//Another rand bool to check whether to slightly go over the edge of 3x3 or to 1/4 or 3/4 of grid
		if (FMath::RandBool())
		{
			//Go to the bottom 3/4 of the grid
			while (ChangedY != (Grid_Y_Size * 3) / 4)
			{
				GO_DOWN(ChangedX, ChangedY);
			}
		}
		else
		{
			//go to the bottom edge of the 3x3 spawn area
			while (ChangedY != (Grid_Y_Size / 2) + Spawn_Deadzone + 1)
			{
				GO_DOWN(ChangedX, ChangedY);
			}
		}
		*/
		
	}
	else //going Up
	{
		// Choose between the top quarter of the grid or the top edge of the spawn area
		int TargetY = FMath::RandBool()
		? Grid_Y_Size / 4
		: (Grid_X_Size / 2) - Spawn_Deadzone - 1;

		while (ChangedY != TargetY)
		{
			GO_UP(ChangedX, ChangedY);
		}

		/*
		//Another rand bool to check whether to slightly go over the edge of 3x3 or to 1/4 or 3/4 of grid
		if (FMath::RandBool())
		{
			//Go to the top 1/4 y pos of the grid 
			while (ChangedY != Grid_Y_Size / 4)
			{
				GO_UP(ChangedX, ChangedY);
			}
		}
		else
		{
			//go to the top most edge of the 3x3 spawn
			while (ChangedY != (Grid_X_Size / 2) - Spawn_Deadzone - 1)
			{
				GO_UP(ChangedX, ChangedY);
			}
		}
		*/
	}

	// Step 2: Move horizontally to reach other side of spawn

	if (bStartedFromLeft) //do this if started from left side of 3x3 spawn
	{
		//Once done with choosing to go up or down to bypass 3x3 spawn, now go around it
		while (ChangedX != (Grid_X_Size / 2) + Spawn_Deadzone + 1)
		{
			GO_RIGHT(ChangedX, ChangedY);
		}
	}
	else //Do this to go around when starting from the right
	{
		while (ChangedX != (Grid_X_Size / 2) - Spawn_Deadzone - 1)
		{
			GO_LEFT(ChangedX, ChangedY);
		}
	}
}




void AHousegeon_Game_Base::GO_LEFT(int& ChangedX, int UnMovedY)
{
	UE_LOG(LogTemp, Display, TEXT("Going left"));
	//Move the X by one to the left First check if the new x is valid within the array, particularly when 0
	//Don't want to change to -1 if the valid bounds check fails in code below
	int NewX = ChangedX - 1;

	//First check if the place you will be moving to is within bounds of the grid array
	if (DungeonGridInfo.IsValidIndex(NewX))
	{
		//Double check that the column inputted is valid
		if (DungeonGridInfo[NewX].IsValidIndex(UnMovedY))
		{
			//Now that I am comfortable every bound is checked, I can now set the inputted x to the new x
			ChangedX = NewX;

			//Then check if the place moving to is a wall, do this because POIs are assumed to have a 1x1 perimeter,
			//Meaning that the player can still walk through it (I don't want to change a POI to a floor)
			if (DungeonGridInfo[ChangedX][UnMovedY] == EDungeonGenerationType::Wall) 
			{
				//Change the wall into a walkable cell
				DungeonGridInfo[ChangedX][UnMovedY] = EDungeonGenerationType::Floor;
			}
		}
	}
}

void AHousegeon_Game_Base::GO_RIGHT(int& ChangedX, int UnMovedY)
{
	UE_LOG(LogTemp, Display, TEXT("Going Right"));
	//Move the X by one to the Right. First check if the new x is valid within the array, particularly when too big a num
	//Don't want to change to array max if the valid bounds check fails in code below
	int NewX = ChangedX + 1;

	//First check if the place you will be moving to is within bounds of the grid array
	if (DungeonGridInfo.IsValidIndex(NewX))
	{
		//Double check that the column inputted is valid
		if (DungeonGridInfo[NewX].IsValidIndex(UnMovedY))
		{
			//Now that I am comfortable every bound is checked, I can now set the inputted x to the new x
			ChangedX = NewX;

			//Then check if the place moving to is a wall, do this because POIs are assumed to have a 1x1 perimeter,
			//Meaning that the player can still walk through it (I don't want to change a POI to a floor)
			if (DungeonGridInfo[ChangedX][UnMovedY] == EDungeonGenerationType::Wall)
			{
				//Change the wall into a walkable cell
				DungeonGridInfo[ChangedX][UnMovedY] = EDungeonGenerationType::Floor;
			}
		}
	}
}

void AHousegeon_Game_Base::GO_UP(int UnMovedX, int& ChangedY)
{
	UE_LOG(LogTemp, Display, TEXT("Going Up"));
	//Move the Y by one to the UP. Don't set ChangedY yet because need to check if is in bounds
	int NewY = ChangedY - 1;

	//First check if the inputted row is actually valid
	if (DungeonGridInfo.IsValidIndex(UnMovedX))
	{
		//Check that the new column moved to is within bounds of the grid array (Column is the Y being traversed)
		if (DungeonGridInfo[UnMovedX].IsValidIndex(NewY))
		{
			//Now that I am comfortable every bound is checked, I can now set the inputted y to the new y
			ChangedY = NewY;

			//Then check if the place moving to is a wall, do this because POIs are assumed to have a 1x1 perimeter,
			//Meaning that the player can still walk through it (I don't want to change a POI to a floor)
			if (DungeonGridInfo[UnMovedX][ChangedY] == EDungeonGenerationType::Wall)
			{
				//Change the wall into a walkable cell
				DungeonGridInfo[UnMovedX][ChangedY] = EDungeonGenerationType::Floor;
			}
		}
	}
}

void AHousegeon_Game_Base::GO_DOWN(int UnMovedX, int& ChangedY)
{
	UE_LOG(LogTemp, Display, TEXT("Going Down"));
	//Move the Y by one Down. Don't set ChangedY yet because need to check if is in bounds
	int NewY = ChangedY + 1;

	//First check if the inputted row is actually valid
	if (DungeonGridInfo.IsValidIndex(UnMovedX))
	{
		//Check that the new column moved to is within bounds of the grid array (Column is the Y being traversed)
		if (DungeonGridInfo[UnMovedX].IsValidIndex(NewY))
		{
			//Now that I am comfortable every bound is checked, I can now set the inputted y to the new y
			ChangedY = NewY;

			//Then check if the place moving to is a wall, do this because POIs are assumed to have a 1x1 perimeter,
			//Meaning that the player can still walk through it (I don't want to change a POI to a floor)
			if (DungeonGridInfo[UnMovedX][ChangedY] == EDungeonGenerationType::Wall)
			{
				//Change the wall into a walkable cell
				DungeonGridInfo[UnMovedX][ChangedY] = EDungeonGenerationType::Floor;
			}
		}
	}
}

void AHousegeon_Game_Base::Dungeon_Logic_Finished()
{
	AGS_DungeonGeneration* DungeonGameState = GetWorld()->GetGameState<AGS_DungeonGeneration>();

	//Now build my custom grid based navigation for ai and player to be able to move along by checking
	//which cells have the floor dungeon generation type

	TArray<TArray<bool>> Temp_NavigationGrid;

	Temp_NavigationGrid.SetNum(Grid_X_Size);

	//Forward declare the spawn locations for the enemies which will be set in the for loop below
	TArray<FIntPoint> SpawnLocationsForEnemies;

	//Forward declare a TempIntPoint which gets overwritten each time when there is a walkable area in the for loop below
	FIntPoint TempIntPoint;

	for (int x = 0; x < Grid_X_Size; x++) 
	{
		Temp_NavigationGrid[x].SetNum(Grid_Y_Size);

		for (int y = 0; y < Grid_Y_Size; y++) 
		{
			//So check the dungeon grid info to see if the same location has a floor, if true, that cell is walkable
			//else it is not (Current scope of project will always have floors as a walkable tile, no current plans to have
			//... walkable POIs) AND spawn counts as a walkable floor as well!
			Temp_NavigationGrid[x][y] =
				(DungeonGridInfo[x][y] == EDungeonGenerationType::Floor)
				|| (DungeonGridInfo[x][y] == EDungeonGenerationType::Spawn);

			//I don't want the enemy to spawn within the spawn bounds of the player so I create a shield around the spawn
			//by checking if the x and y is NOT within bounds of the spawn
			if (!(x > (Grid_X_Size / 2) - Spawn_Deadzone - 1 && x < (Grid_X_Size / 2) + Spawn_Deadzone + 1
				&& y > (Grid_Y_Size / 2) - Spawn_Deadzone - 1 && y < (Grid_Y_Size / 2) + Spawn_Deadzone + 1))
			{
				//Add the x and y to the tempintpoint (Remember that they are int 2dvectors!)
				TempIntPoint.X = x;
				TempIntPoint.Y = y;

				//Then add it into the set which can be randomly selected when spawning the enemies later on
				SpawnLocationsForEnemies.Add(TempIntPoint);
			}
		}
	}

	if (DungeonGameState) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Found the dungeon generation game state!"));

		DungeonGameState->Set_Dungeon_Grid_Info(DungeonGridInfo);
		DungeonGameState->NavigationGrid = Temp_NavigationGrid;
	}
	else 
	{
		UE_LOG(LogTemp, Error, TEXT("NOT Found the dungeon generation game state!"));
	}

	Spawn_Enemies(SpawnLocationsForEnemies);
}

void AHousegeon_Game_Base::Spawn_Enemies(TArray<FIntPoint>  SpawnLocationsForEnemies)
{
	//If the SpawnLocations are empty, instantly return and stop all logic from below
	if (SpawnLocationsForEnemies.IsEmpty()) return;

	//Forward declare the variables to be used in the for loop below
	FIntPoint TempIntPoint;
	int RandIndex = 0;
	TSubclassOf<AEnemy> EnemyClassToSpawn;
	//Have the spawn transform, (The random spawn rotation will be handled within the AI controller or enemy as I need a
	//Normalised Yaw value)
	FTransform SpawnTransform;
	FVector SpawnLocation;

	//Get the spawn enemy array from the specific level that the game is on
	TArray<TSubclassOf<AEnemy>> LevelSpecificEnemiesToSpawn = Retrieve_Enemy_Array_From_Current_Level();

	//Do a for loop for the amount of enemies to spawn, this is from a global property
	for (int i = 0; i < AmountOfEnemiesToSpawn; i++) 
	{
		//Randomally choose a spawn location for the enemy by retriving a random index from the Set array
		RandIndex = FMath::RandRange(0, SpawnLocationsForEnemies.Num() - 1);

		//Set the tempintpoint by retrieving it from the potential spawn location array
		TempIntPoint = SpawnLocationsForEnemies[RandIndex];

		//Then remove the spawn location from the array so no other enemy can spawn in the same location
		SpawnLocationsForEnemies.RemoveAt(RandIndex);

		//Now randomally choose which enemy to spawn in the location by picking a random index from the enemy spawn array
		//(Reusing the RandIndex variable)
		RandIndex = FMath::RandRange(0, LevelSpecificEnemiesToSpawn.Num() - 1);
		EnemyClassToSpawn = LevelSpecificEnemiesToSpawn[RandIndex];

		//Then I need to set the location to where the enemy would be in world location (Each cell = 400cm3, starting from
		//0,0)
		SpawnLocation.X = TempIntPoint.X * 400.f;
		SpawnLocation.Y = TempIntPoint.Y * 400.f;
		//The z location will be set within the enemy class as I expect them to have different heights, BUT will encase the
		//400x400 x/y size
		SpawnTransform.SetLocation(SpawnLocation);

		//With everything being picked, now spawn the actor at the location
		GetWorld()->SpawnActor<AEnemy>(EnemyClassToSpawn, SpawnTransform);
	}
}

TArray<TSubclassOf<AEnemy>> AHousegeon_Game_Base::Retrieve_Enemy_Array_From_Current_Level()
{
	//Go through a switch case from the current level int property (Set from BP edit screen or by going from one level to
	//another) to return the enemies to spawn for that level
	switch (CurrentLevel) 
	{
	case 0:
		return Level1EnemiesToSpawn;
		break;

	default:
		return Level1EnemiesToSpawn;
		break;
	}
}
