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
			//Check if the random spawn is not within a certain bound of the spawn area
			if (!(RandX >= (Grid_X_Size / 2) - 2 && RandX <= (Grid_X_Size / 2) + 2
				&& RandY >= (Grid_Y_Size / 2) - 2 && RandY <= (Grid_Y_Size / 2) + 2)) 
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

	//Path moved from center up, down, left right (Randomly)
	EPath_Moved PathMoved; 

	//I go through all the end locations and then create paths for all of them
	for (int i = 0; i < MyEndLocations.Num(); i++) 
	{
		//Start from the center of the grid
		Player_Start_Row = Grid_X_Size / 2;
		Player_Start_Column = Grid_Y_Size / 2;

		int Player_Start_Random = FMath::RandRange(0, 3); //Random start location
		Player_Start_Random = 2; //Debugging Number Thing

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

		//Randomly pick if being Row Traversal, set as a variable for help whilst debugging the patterns
		bool bRowTraversalFirst = FMath::RandBool();
		bRowTraversalFirst = true;

		if (bRowTraversalFirst)
		{
			UE_LOG(LogTemp, Display, TEXT("Picked Row Traversal First For Path Pattern!"));

			switch (PathMoved) 
			{
			case EPath_Moved::LEFT:
				SpawnedLeft_RowFirst_LineTraversal(Player_Start_Row, Player_Start_Column,
					MyEndLocations[i].X, MyEndLocations[i].Y);
				break;

			case EPath_Moved::RIGHT:
				SpawnedRight_RowFirst_LineTraversal(Player_Start_Row, Player_Start_Column,
					MyEndLocations[i].X, MyEndLocations[i].Y);
				break;

			case EPath_Moved::UP:
				SpawnedUpDown_RowFirst_LineTraversal(true, Player_Start_Row, Player_Start_Column,
					MyEndLocations[i].X, MyEndLocations[i].Y);
				break;

			case EPath_Moved::DOWN:
				SpawnedUpDown_RowFirst_LineTraversal(false, Player_Start_Row, Player_Start_Column,
					MyEndLocations[i].X, MyEndLocations[i].Y);
				break;
			}
		}
		else 
		{
			UE_LOG(LogTemp, Display, TEXT("Picked Column Traversal First For Path Pattern!"));
		}
		


	}
}

void AHousegeon_Game_Base::SpawnedLeft_RowFirst_LineTraversal(int StartX, int StartY, int EndX, int EndY)
{
	// Validate bounds, if not, return which stops all logic from below from happening
	if (!DungeonGridInfo.IsValidIndex(StartX) || !DungeonGridInfo.IsValidIndex(EndX)) return;
	if (!DungeonGridInfo[StartX].IsValidIndex(StartY) || !DungeonGridInfo[EndX].IsValidIndex(EndY)) return;

	//if the ax is less than bx, that means that you have to go right
	if (StartX < EndX)
	{
		//I go left one more time because if I am going around the spawn, I want a wall's length around the spawn
		//Whilst, looping around it
		GO_LEFT(StartX, StartY);

		//This is a bit different to the others, I don't want to go back through spawn
		//This means that I have to go around the spawn by going up or down first and then doing Row traversal
		//Choose randomly to go up or down because it does not need to be accurate as creating weird confusing
		//Paths is the point
		Go_Around_Spawn(StartX, StartY);

		while (StartX != EndX)
		{
			//Keep going right until start x and end x are equal
			GO_RIGHT(StartX, StartY);
		}
	}
	else if (StartX > EndX)//Going left if StartX is bigger than EndX
	{
		while (StartX != EndX)
		{
			GO_LEFT(StartX, StartY);
		}
	}

	if (StartY > EndY) //If Start Y is bigger means that it is below meaning that it has to go up
	{
		while (StartY != EndY)
		{
			GO_UP(StartX, StartY);
		}
	}
	else if (StartY < EndY) //If the Start Y is less, means that it is on top of End Y and needs to go down
	{
		while (StartY != EndY)
		{
			GO_DOWN(StartX, StartY);
		}
	}
}

void AHousegeon_Game_Base::SpawnedRight_RowFirst_LineTraversal(int StartX, int StartY, int EndX, int EndY)
{
	// Validate bounds, if not, return which stops all logic from below from happening
	if (!DungeonGridInfo.IsValidIndex(StartX) || !DungeonGridInfo.IsValidIndex(EndX)) return;
	if (!DungeonGridInfo[StartX].IsValidIndex(StartY) || !DungeonGridInfo[EndX].IsValidIndex(EndY)) return;

	//if the ax is less than bx, that means that you have to go right
	if (StartX < EndX)
	{
		//Since you already spawned from the right, you don't have to go around spawn
		while (StartX != EndX)
		{
			//Keep going right until start x and end x are equal
			GO_RIGHT(StartX, StartY);
		}
	}
	else if (StartX > EndX)//Going left if StartX is bigger than EndX
	{
		//I go right one more time because if I am going around the spawn, I want a wall's length around the spawn
		//Whilst, looping around it
		GO_RIGHT(StartX, StartY);

		//This is a bit different to the others, I don't want to go back through spawn
		//This means that I have to go around the spawn by going up or down first and then doing Row traversal
		//Choose randomly to go up or down because it does not need to be accurate as creating weird confusing
		//Paths is the point
		Go_Around_Spawn(StartX, StartY);
		while (StartX != EndX)
		{
			GO_LEFT(StartX, StartY);
		}
	}

	if (StartY > EndY) //If Start Y is bigger means that it is below meaning that it has to go up
	{
		while (StartY != EndY)
		{
			GO_UP(StartX, StartY);
		}
	}
	else if (StartY < EndY) //If the Start Y is less, means that it is on top of End Y and needs to go down
	{
		while (StartY != EndY)
		{
			GO_DOWN(StartX, StartY);
		}
	}
}

void AHousegeon_Game_Base::SpawnedUp_RowFirst_LineTraversal(int StartX, int StartY, int EndX, int EndY)
{
	// Validate bounds, if not, return which stops all logic from below from happening
	if (!DungeonGridInfo.IsValidIndex(StartX) || !DungeonGridInfo.IsValidIndex(EndX)) return;
	if (!DungeonGridInfo[StartX].IsValidIndex(StartY) || !DungeonGridInfo[EndX].IsValidIndex(EndY)) return;

	//First off, move one more up, because going horizontal first, meaning I don't want the 3x3 spawn to look ugly
	//When going left to right
	GO_UP(StartX, StartY);

	//First check if the end x is within bounds of the row span of the spawn 3x3 and if it is down from the start y
	//Do this because, you will have to go completely around the 3x3 so that you can not degrade the beauty of
	//the 3x3 spawn square. Can be dangerous if the grid is not a perfect square, but my whole code assumes it is
	//If the game changes to have irregular grids, will change
	if (EndX >= (Grid_X_Size / 2) - 2 && EndX <= (Grid_X_Size / 2) + 2
		&& EndY > StartY + 1) //Do a plus one because I moved one up already and I know this is on edges of 3x3
	{
		//An updated version to go around spawn when you want to to a horizontal first line
		Go_Around_Spawn_Vertical_Rated_Version(true, StartX, StartY);
	}

	//if the ax is less than bx, that means that you have to go right
	if (StartX < EndX)
	{
		//Since you already spawned from the right, you don't have to go around spawn
		while (StartX != EndX)
		{
			//Keep going right until start x and end x are equal
			GO_RIGHT(StartX, StartY);
		}
	}
	else if (StartX > EndX)//Going left if StartX is bigger than EndX
	{
		while (StartX != EndX)
		{
			GO_LEFT(StartX, StartY);
		}
	}

	if (StartY > EndY) //If Start Y is bigger means that it is below meaning that it has to go up
	{
		while (StartY != EndY)
		{
			GO_UP(StartX, StartY);
		}
	}
	else if (StartY < EndY) //If the Start Y is less, means that it is on top of End Y and needs to go down
	{
		while (StartY != EndY)
		{
			GO_DOWN(StartX, StartY);
		}
	}
}

void AHousegeon_Game_Base::SpawnedUpDown_RowFirst_LineTraversal(bool bStartedFromUp, int StartX, int StartY, int EndX, int EndY)
{
	//This function is more simple than the spawned left/right because there is more space for the paths to
	//travel when the x axis is not being blocked by the center spawn

	// Validate bounds, if not, return which stops all logic from below from happening
	if (!DungeonGridInfo.IsValidIndex(StartX) || !DungeonGridInfo.IsValidIndex(EndX)) return;
	if (!DungeonGridInfo[StartX].IsValidIndex(StartY) || !DungeonGridInfo[EndX].IsValidIndex(EndY)) return;

	//Forward declare variables
	int Y_OffsetCheckerForMoveAroundSpawn;
	bool bDoGoAround;

	//First off, move one more down or up, because going horizontal first, meaning I don't want the 3x3 spawn to look ugly
	//When going left to right
	if (bStartedFromUp) 
	{
		GO_UP(StartX, StartY);
		//Since you just moved up, you need to have an offset to go down by one when checking the move around bounds below
		Y_OffsetCheckerForMoveAroundSpawn = +1;

		//The bounds logic is used for if below
		bDoGoAround = (EndX >= (Grid_X_Size / 2) - 2 && EndX <= (Grid_X_Size / 2) + 2
			&& EndY > StartY + Y_OffsetCheckerForMoveAroundSpawn);
	}
	else 
	{
		GO_DOWN(StartX, StartY);
		//Since you just moved down, you need to have an offset to go up by one when checking the move around bounds below
		Y_OffsetCheckerForMoveAroundSpawn = -1;

		//The bounds logic is used for if below
		bDoGoAround = (EndX >= (Grid_X_Size / 2) - 2 && EndX <= (Grid_X_Size / 2) + 2
			&& EndY < StartY + Y_OffsetCheckerForMoveAroundSpawn);
	}
	//Explanation for below above ^^^^^^^^ (Moved so this could be used as an up/down function instead of seperately)
	//First check if the end x is within bounds of the row span of the spawn 3x3 and if it is up one from the start y
	//Do this because, you will have to go completely around the 3x3 so that you can not degrade the beauty of
	//the 3x3 spawn square. Can be dangerous if the grid is not a perfect square, but my whole code assumes it is
	//If the game changes to have irregular grids, will change
	if (bDoGoAround) //Do a -/+ one because I moved one down/up already and I know this is on edges of 3x3
	{
		//An updated version to go around spawn when you want to to a horizontal first line
		Go_Around_Spawn_Vertical_Rated_Version(bStartedFromUp, StartX, StartY);
	}

	while (StartX != EndX) //Move horizontally first
	{
		//if the start x is less than end x, means that it is below and needs to be moved right
		//the else is the inverse
		//Once it is equal, meaning, you reached the coord, the while loop breaks because it reached the condition
		(StartX < EndX) ? GO_RIGHT(StartX, StartY) : GO_LEFT(StartX, StartY);
	}

	while (StartY != EndY) //Move vertically next
	{
		//if the start y is bigger than end y, means that it is below and needs to be moved up
		//the else is the inverse
		//Once it is equal, meaning, you reached the coord, the while loop breaks because it reached the condition
		(StartY > EndY)? GO_UP(StartX, StartY) : GO_DOWN(StartX, StartY);
	}

	/*
	//if the ax is less than bx, that means that you have to go right
	if (StartX < EndX)
	{
		//Since you already spawned from the right, you don't have to go around spawn
		while (StartX != EndX)
		{
			//Keep going right until start x and end x are equal
			GO_RIGHT(StartX, StartY);
		}
	}
	else if (StartX > EndX)//Going left if StartX is bigger than EndX
	{
		while (StartX != EndX)
		{
			GO_LEFT(StartX, StartY);
		}
	}

	if (StartY > EndY) //If Start Y is bigger means that it is below meaning that it has to go up
	{
		while (StartY != EndY)
		{
			GO_UP(StartX, StartY);
		}
	}
	else if (StartY < EndY) //If the Start Y is less, means that it is on top of End Y and needs to go down
	{
		while (StartY != EndY)
		{
			GO_DOWN(StartX, StartY);
		}
	}
	*/
	
}

void AHousegeon_Game_Base::Go_Around_Spawn(int& ChangedX, int& ChangedY)
{
	//Helper function to go around the 3x3 spawn to make the paths look better
	if (FMath::RandBool())
	{
		//if the end x,y is kinda in the center of the grid, the path will go up and then right
		//Then go through the spawn, image inside my head makes it look cool

		for (int i = 0; i < 3; i++)
		{
			GO_UP(ChangedX, ChangedY);
		}

		//I choose 3 ups and downs because the spawn area is 3x3 and always will be (if needs to change, I'll make
		//Variable)
	}
	else
	{
		//if the end x,y is kinda in the center of the grid, the path will go down and then right
		//Then go through the spawn, image inside my head makes it look cool
		for (int i = 0; i < 3; i++)
		{
			GO_DOWN(ChangedX, ChangedY);
		}
	}
}

void AHousegeon_Game_Base::Go_Around_Spawn_Vertical_Rated_Version(bool bStartedFromUp, int& ChangedX, int& ChangedY)
{
	//Randomly choose between going horizontlly right 3 times or left before going down
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
		//Then you have to go 7 down on y because: 3 = spawn, 2 = moving up, 2 = the gap to make beauty spawn
		for (int i = 0; i < 7; i++) 
		{
			GO_DOWN(ChangedX, ChangedY);
		}

	}
	else //So you started from down, do the inverse of above
	{
		//Then you have to go 7 UP on y because: 3 = spawn, 2 = moving down, 2 = the gap to make beauty spawn
		for (int i = 0; i < 7; i++)
		{
			GO_UP(ChangedX, ChangedY);
		}
	}
}




void AHousegeon_Game_Base::GO_LEFT(int& ChangedX, int UnMovedY)
{
	UE_LOG(LogTemp, Display, TEXT("Going left"));
	//Move the X by one to the left
	ChangedX -= 1;

	//First check if the place you will be moving to is within bounds of the grid array
	if (DungeonGridInfo.IsValidIndex(ChangedX)) 
	{
		//Double check that the column inputted is valid
		if (DungeonGridInfo[ChangedX].IsValidIndex(UnMovedY)) 
		{
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
	//Move the X by one to the Right
	ChangedX += 1;

	//First check if the place you will be moving to is within bounds of the grid array
	if (DungeonGridInfo.IsValidIndex(ChangedX))
	{
		//Double check that the column inputted is valid
		if (DungeonGridInfo[ChangedX].IsValidIndex(UnMovedY))
		{
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
	//Move the Y by one to the UP
	ChangedY -= 1;

	//First check if the inputted row is actually valid
	if (DungeonGridInfo.IsValidIndex(UnMovedX))
	{
		//Check that the new column moved to is within bounds of the grid array (Column is the Y being traversed)
		if (DungeonGridInfo[UnMovedX].IsValidIndex(ChangedY))
		{
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
	//Move the Y by one to the UP
	ChangedY += 1;

	//First check if the inputted row is actually valid
	if (DungeonGridInfo.IsValidIndex(UnMovedX))
	{
		//Check that the new column moved to is within bounds of the grid array (Column is the Y being traversed)
		if (DungeonGridInfo[UnMovedX].IsValidIndex(ChangedY))
		{
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
