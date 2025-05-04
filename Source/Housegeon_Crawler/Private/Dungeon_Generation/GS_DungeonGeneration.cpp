// Fill out your copyright notice in the Description page of Project Settings.
#include "Dungeon_Generation/GS_DungeonGeneration.h"



void AGS_DungeonGeneration::Set_Dungeon_Grid_Info(TArray<TArray<EDungeonGenerationType>> DungeonGridInfo_Param)
{
	DungeonGridInfo = DungeonGridInfo_Param;

	if (DungeonGridInfo.IsValidIndex(0)) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Dungeon Grid ready to be broadcasted for generation!"));
		OnGridReady.Broadcast();
	}
	else 
	{
		UE_LOG(LogTemp, Error, TEXT("Dungeon Grid Not ready to be broadcasted for generation cause invalid array!"));
	}
}

TArray<TArray<EDungeonGenerationType>> AGS_DungeonGeneration::Get_Dungeon_Grid_Info()
{
	return DungeonGridInfo;
}
