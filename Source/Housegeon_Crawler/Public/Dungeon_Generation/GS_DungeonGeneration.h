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

	/*
	//Registers the inputted cell location as not movable and adds the entity to the damage cell array, Don't use if moving
	//only use for when the entity has spawned or has not moved from one cell to another!.
	*/
	void Register_Entity_Cell_Location(FIntPoint EntityCellInfo, AActor* EntityToRegister);

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
	void Moving_Forward(AActor* EntityMoved, int& StartX, int& StartY, float CurrentYaw);

	void Try_Sending_Damage_To_Entity(TArray<FIntPoint> DamageCells, float Damage);

	/*
		So you killed an entity, before destroying, call this function to update the cell to be movable and other stuff
	*/
	void Killed_An_Entity(FIntPoint CellLocation);

private:

	FIntPoint CurrentPlayerCoords;

	TMap<FIntPoint, TWeakObjectPtr<AActor>> EntityCoords;

	AAI_Manager* myAIManager;

	//The 2d array that holds all of the spawn logic when it has to be displayed by HISMs later
	TArray<TArray<EDungeonGenerationType>> DungeonGridInfo;
};
