// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "Dungeon_Generation/EDungeonGenerationType.h"
#include "Dungeon_Generation/EPathTraversalType.h"
#include "Dungeon_Generation/EAxisTraversalChoice.h"

#include "Dungeon_Generation/FPathSpawnAreaChoice.h"

#include "Dungeon_Generation/FEnemySpawnData.h"

#include "Housegeon_Game_Base.generated.h"

//Create a struct that holds all of the end points for the path traversal when making the paths
//This will be held within an array
struct FEnd_Location_Data
{
	int X;
	int Y;
};

//This enum is responsible for path creation, The player will start in the middle of the grid and then
//A random path will be selected starting from the choices below and go from the direction selected
enum class EPath_Moved
{
	UP,
	RIGHT,
	DOWN,
	LEFT
};

/**
 * 
 */
UCLASS()
class HOUSEGEON_CRAWLER_API AHousegeon_Game_Base : public AGameModeBase
{
	GENERATED_BODY()

protected:

	virtual void StartPlay();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++ Generation Logic")
	int Grid_X_Size = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++ Generation Logic")
	int Grid_Y_Size = 10;

	//When objects are being generated for the dungeon, have a forcefield offset from where they can spawn from the player spawn area
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "C++ Generation Logic")
	int Spawn_Deadzone = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Spawn Logic")
	TArray<TSubclassOf<AEnemy>> Level1EnemiesToSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon POIs")
	int Amount_Of_Dead_Ends = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon POIs")
	int Amount_Of_Chest_POIs = 1;

	UFUNCTION(BlueprintCallable, Category = "C++ Functions")
	void Generate_Dungeon(EPathTraversalType TraversalType, EAxisTraversalChoice AxisType, FPathSpawnAreaChoice PathSpawn);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Options")
	int CurrentLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Options")
	int AmountOfEnemiesToSpawn = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Options")
	EPathTraversalType ChoiceOfTraversal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Options")
	EAxisTraversalChoice WhichAxisFirst;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Options")
	FPathSpawnAreaChoice WherePathSpawns;
	
private:

	//The 2d array that holds all of the spawn logic when it has to be displayed by HISMs later
	TArray<TArray<EDungeonGenerationType>> DungeonGridInfo;

	//Create all of the points for the paths to go to
	void Create_Starter_Walls();
	
	void Create_Spawn_To_End();

	void Create_Dead_End(int Amount);
	void Create1x1_ChestPOI(int Amount);

	void Set_Random_End_XY(int& EndX, int& EndY);

	//Dungeon path creation logic, picks the type of path traversal to reach from a to b
	void Create_Path_From_Start_To_End();
	EPath_Moved Get_Random_Spawn_Point();
	void Pick_Random_Row_Traversal(EPath_Moved PathMoved, bool bLineTraversal, int Player_Start_Row, int Player_Start_Column,
		int EndX, int EndY);
	void Pick_Random_Column_Traversal(EPath_Moved PathMoved, bool bLineTraversal, int Player_Start_Row, int Player_Start_Column,
		int EndX, int EndY);


	//ROW TRAVERSAL:
	//Path Pattern Traversal Row First, Travel on the X first and then Y
	void SpawnedLeftRight_RowFirst_LineTraversal(bool bStartedFromLeft, int StartX, int StartY, int EndX, int EndY);
	void SpawnedUpDown_RowFirst_LineTraversal(bool bStartedFromUp, int StartX, int StartY, int EndX, int EndY);
	//Path Pattern Row First Stair Traversal (Go x amount on each axis before reaching end point)
	void SpawnedLeft_RowFirst_StairTraversal(int StartX, int StartY, int EndX, int EndY, int X_Increment = 2, int Y_Increment = 2);
	void SpawnedRight_RowFirst_StairTraversal(int StartX, int StartY, int EndX, int EndY, int X_Increment = 2, int Y_Increment = 2);
	void SpawnedUpDown_RowFirst_StairTraversal(bool bStartedFromUp, int StartX, int StartY, int EndX, int EndY, int X_Increment = 2, int Y_Increment = 2);

	//The Path Algorithms
	void DO_RowFirst_LineAlgorithm(int StartX, int StartY, int EndX, int EndY);
	void DO_RowFirst_StairAlgorithm(int StartX, int StartY, int EndX, int EndY, int X_Increment = 2, int Y_Increment = 2);
	void DO_ColumnFirst_LineAlgorithm(int StartX, int StartY, int EndX, int EndY);
	void DO_ColumnFirst_StairAlgorithm(int StartX, int StartY, int EndX, int EndY, int X_Increment = 2, int Y_Increment = 2);

	//COLUMN TRAVERSAL:
	//The column first path traversals, travel along the y first and then the x to reach coord
	void SpawnedLeftRight_ColumnFirst_LineTraversal(bool bStartedFromLeft, int StartX, int StartY, int EndX, int EndY);
	void SpawnedUpDown_ColumnFirst_LineTraversal(bool bStartedFromUp, int StartX, int StartY, int EndX, int EndY);
	//Path Pattern Column First Stair Traversal (Go x amount on each axis before reaching end point but starting with column) 
	void SpawnedUp_ColumnFirst_StairTraversal(int StartX, int StartY, int EndX, int EndY, int X_Increment = 2, int Y_Increment = 2);
	void SpawnedDown_ColumnFirst_StairTraversal(int StartX, int StartY, int EndX, int EndY, int X_Increment = 2, int Y_Increment = 2);
	void SpawnedLeftRight_ColumnFirst_StairTraversal(bool bStartedFromLeft, int StartX, int StartY, int EndX, int EndY, int X_Increment = 2, int Y_Increment = 2);

	//The helper functions to go around the 3x3 spawns for the traversal algorithms
	//Row Based Helper Functions
	void Go_Around_Spawn_RowBased_LeftRight(int& ChangedX, int& ChangedY, int EndX, int EndY);
	void Go_Around_Spawn_Vertical_Rated_Version(bool bStartedFromUp, int& ChangedX, int& ChangedY);
	void Go_Around_Spawn_VerticalStair_Rated_Version(bool bStartedFromUp, int& ChangedX, int& ChangedY);

	//Column Based Helper functions
	void Go_Around_Spawn_ColumnBased_LeftRight(bool bStartedFromLeft, int& ChangedX, int& ChangedY);
	void Go_Around_Spawn_ColumnBased_UpDown(int& ChangedX, int& ChangedY, int EndX, int EndY);
	void Go_Around_Spawn_ColumnBasedStairs_LeftRight(bool bStartedFromLeft, int& ChangedX, int& ChangedY);

	//Actually Create The Floor Path When Moving From The Path Patterns
	void GO_LEFT(int &ChangedX, int UnMovedY);
	void GO_RIGHT(int& ChangedX, int UnMovedY);
	void GO_UP(int UnMovedX, int &ChangedY);
	void GO_DOWN(int UnMovedX, int& ChangedY);

	//(In The Header) every function below will deal with the AI systems by creating the navigation and spawning of enemies
	void Dungeon_Logic_Finished();

	void Spawn_Enemies(TArray<FIntPoint>  SpawnLocationsForEnemies);

	/*
		Retrieve the enemy arrays, I use individual Tarrays for each level because BP does not support nested containers
		so this is a bandaid solution to give designers more freedom when moving around enemies to spawn for each level
	*/
	TArray<TSubclassOf<AEnemy>> Retrieve_Enemy_Array_From_Current_Level();


	//Will hold all the end locations when making paths from a to b
	TArray<FEnd_Location_Data> MyEndLocations;
};
