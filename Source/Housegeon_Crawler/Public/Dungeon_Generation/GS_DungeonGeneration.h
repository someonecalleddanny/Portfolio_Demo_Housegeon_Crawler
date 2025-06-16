// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"

#include "Dungeon_Generation/EDungeonGenerationType.h"

#include "Math/IntPoint.h"

#include "Managers/AI_Manager.h"

#include "GS_DungeonGeneration.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGridReady);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAIReady);
/**
 * 
 */
UCLASS()
class HOUSEGEON_CRAWLER_API AGS_DungeonGeneration : public AGameStateBase
{
	GENERATED_BODY()
public:
	void Set_Dungeon_Grid_Info(TArray<TArray<EDungeonGenerationType>> DungeonGridInfo_Param, TArray<TArray<bool>> NavigationGrid_Param);

	//This function updates the global player coords (Used for AI) as well as setting the cell as not movable onto
	void SetPlayerSpawnInformation(FIntPoint PlayerCellInfo);

	void Register_Entity_Cell_Location(FIntPoint EntityCellInfo);

	void Set_AI_Manager(AAI_Manager* AIManager_PARAM, int MaxSpawnedEntities);

	void Notify_AI_Manager_Patrol_Batch(FAIManagerBatchPacket BatchPacketToSend);

	//Make the cell that the entity moved from movable to all entities so that they can move onto that cell
	void UpdateOldMovementCell(FIntPoint CellInfo);

	FIntPoint GetPlayerCoords();

	void UpdatePlayerCoords(AActor* PlayerRef, FIntPoint NewCoords);

	TArray<TArray<EDungeonGenerationType>> Get_Dungeon_Grid_Info();

	TArray<TArray<bool>> NavigationGrid;

	FOnGridReady OnGridReady;
	FOnAIReady OnAIManagerReady;

	bool Can_Move_Forward(int StartX, int StartY, float CurrentYaw);
	void Moving_Forward(int& StartX, int &StartY, float CurrentYaw);

private:

	FIntPoint CurrentPlayerCoords;

	AAI_Manager* myAIManager;

	//The 2d array that holds all of the spawn logic when it has to be displayed by HISMs later
	TArray<TArray<EDungeonGenerationType>> DungeonGridInfo;
};
