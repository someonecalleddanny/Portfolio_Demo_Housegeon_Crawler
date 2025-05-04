// Fill out your copyright notice in the Description page of Project Settings.


#include "Housegeon_Game_Base.h"

#include "Dungeon_Generation/GS_DungeonGeneration.h"

void AHousegeon_Game_Base::StartPlay()
{
	Super::StartPlay();
	//Start the dungeon generation by creating the walls which will then be replaced once generation starts
	//(This function sets the size of DungeonGridInfo)
	Create_Starter_Walls();

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
