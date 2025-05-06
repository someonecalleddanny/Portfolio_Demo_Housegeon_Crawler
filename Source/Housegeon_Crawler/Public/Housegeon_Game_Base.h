// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "Dungeon_Generation/EDungeonGenerationType.h"

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon POIs")
	int Amount_Of_Dead_Ends = 4;

	UFUNCTION(BlueprintCallable, Category = "C++ Functions")
	void Generate_Dungeon();
	
private:

	//The 2d array that holds all of the spawn logic when it has to be displayed by HISMs later
	TArray<TArray<EDungeonGenerationType>> DungeonGridInfo;

	void Create_Starter_Walls();
	
	void Create_Spawn_To_End();

	void Create_Dead_End(int Amount);

	void Set_Random_End_XY(int& EndX, int& EndY);

	//Dungeon path creation logic
	void Create_Path_From_Start_To_End();

	//Path Pattern Traversal Row First, Travel on the X first and then Y
	void SpawnedLeft_RowFirst_LineTraversal(int StartX, int StartY, int EndX, int EndY);
	void SpawnedRight_RowFirst_LineTraversal(int StartX, int StartY, int EndX, int EndY);
	void SpawnedUp_RowFirst_LineTraversal(int StartX, int StartY, int EndX, int EndY);
	void SpawnedUpDown_RowFirst_LineTraversal(bool bStartedFromUp, int StartX, int StartY, int EndX, int EndY);

	//Path Pattern Row First: Stair Traversal (Go x amount on each axis before reaching end point)
	void SpawnedLeft_RowFirst_StairTraversal(int StartX, int StartY, int EndX, int EndY, int X_Increment = 2, int Y_Increment = 2);
	void SpawnedRight_RowFirst_StairTraversal(int StartX, int StartY, int EndX, int EndY, int X_Increment = 2, int Y_Increment = 2);

	void DO_RowFirst_StairAlgorithm(int StartX, int StartY, int EndX, int EndY, int X_Increment = 2, int Y_Increment = 2);

	void Go_Around_Spawn(int& ChangedX, int& ChangedY);
	void Go_Around_Spawn_Vertical_Rated_Version(bool bStartedFromUp, int& ChangedX, int& ChangedY);

	//Actually Create The Floor Path When Moving From The Path Patterns
	void GO_LEFT(int &ChangedX, int UnMovedY);
	void GO_RIGHT(int& ChangedX, int UnMovedY);
	void GO_UP(int UnMovedX, int &ChangedY);
	void GO_DOWN(int UnMovedX, int& ChangedY);

	void Dungeon_Logic_Finished();

	//Will hold all the end locations when making paths from a to b
	TArray<FEnd_Location_Data> MyEndLocations;
};
