// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"

#include "Dungeon_Generation/EDungeonGenerationType.h"

#include "GS_DungeonGeneration.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGridReady);

/**
 * 
 */
UCLASS()
class HOUSEGEON_CRAWLER_API AGS_DungeonGeneration : public AGameStateBase
{
	GENERATED_BODY()
public:
	void Set_Dungeon_Grid_Info(TArray<TArray<EDungeonGenerationType>> DungeonGridInfo_Param);

	TArray<TArray<EDungeonGenerationType>> Get_Dungeon_Grid_Info();

	FOnGridReady OnGridReady;

private:
	//The 2d array that holds all of the spawn logic when it has to be displayed by HISMs later
	TArray<TArray<EDungeonGenerationType>> DungeonGridInfo;
};
