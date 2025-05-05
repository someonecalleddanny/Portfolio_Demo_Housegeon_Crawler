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
	
private:

	//The 2d array that holds all of the spawn logic when it has to be displayed by HISMs later
	TArray<TArray<EDungeonGenerationType>> DungeonGridInfo;

	void Create_Starter_Walls();
	
	void Create_Spawn_To_End();

	void Create_Dead_End(int Amount);

	void Set_Random_End_XY(int& EndX, int& EndY);

	//Dungeon path creation logic
	void Create_Path_From_Start_To_End();

	void Dungeon_Logic_Finished();

	//Will hold all the end locations when making paths from a to b
	TArray<FEnd_Location_Data> MyEndLocations;
};
