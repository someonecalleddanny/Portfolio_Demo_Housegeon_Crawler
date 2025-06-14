// Fill out your copyright notice in the Description page of Project Settings.
#include "Dungeon_Generation/GS_DungeonGeneration.h"



void AGS_DungeonGeneration::Set_Dungeon_Grid_Info(TArray<TArray<EDungeonGenerationType>> DungeonGridInfo_Param, TArray<TArray<bool>> NavigationGrid_Param)
{
	DungeonGridInfo = DungeonGridInfo_Param;
    NavigationGrid = NavigationGrid_Param;

    //First check if the dungeon grid info is valid on the x axis, Important because a large part of gameplay relies on this
	if (DungeonGridInfo.IsValidIndex(0)) 
	{
        //If the Y axis of the grid is not valid, then instantly return
        if (!(DungeonGridInfo[0].IsValidIndex(0)))
        {
            UE_LOG(LogTemp, Error, TEXT("Dungeon Grid Y is invalid check Set_Dungeon_Grid_Info function!"));
            return;
        }
		
	}
	else //If not true, instantly return and mark where the error was
	{
        UE_LOG(LogTemp, Error, TEXT("Dungeon Grid X is invalid check Set_Dungeon_Grid_Info function!"));
        return;
	}

    //First check if the NavigationGrid info is valid on the x axis, Important because a large part of gameplay relies on this
    if (NavigationGrid.IsValidIndex(0))
    {
        //If the Y axis of the grid is not valid, then instantly return
        if (!(NavigationGrid[0].IsValidIndex(0)))
        {
            UE_LOG(LogTemp, Error, TEXT("Navigation Grid Y is invalid check Set_Dungeon_Grid_Info function!"));
            return;
        }

    }
    else //If not true, instantly return and mark where the error was
    {
        UE_LOG(LogTemp, Error, TEXT("Navigation Grid X is invalid check Set_Dungeon_Grid_Info function!"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Dungeon/Navigation Grid ready to be broadcasted for generation!"));
    OnGridReady.Broadcast();
}

void AGS_DungeonGeneration::SetPlayerSpawnInformation(FIntPoint PlayerCellInfo)
{
    //First check if the inputted cell info is within bounds to avoid crashing
    if (!(NavigationGrid.IsValidIndex(PlayerCellInfo.X))) 
    {
        UE_LOG(LogTemp, Error, TEXT("The X Spawn Cells where not in range for the navigation grid"));
        UE_LOG(LogTemp, Error, TEXT("The X Input = %d"), PlayerCellInfo.X);
        UE_LOG(LogTemp, Error, TEXT("Current Navigation Grid Size = %d"), NavigationGrid.Num());
        return;
    }
    else if (!(NavigationGrid[PlayerCellInfo.X].IsValidIndex(PlayerCellInfo.Y))) 
    {
        UE_LOG(LogTemp, Error, TEXT("The Y Spawn Cells where not in range for the navigation grid"));
        return;
    }
    //Set the player spawn info by updating the currentplayercoords to the inputted player cell info
    CurrentPlayerCoords = PlayerCellInfo;
    
    //Make the spawn area not movable to any other entities
    NavigationGrid[CurrentPlayerCoords.X][CurrentPlayerCoords.Y] = false;
}

void AGS_DungeonGeneration::Set_AI_Manager(AAI_Manager* AIManager_PARAM, int MaxSpawnedEntities)
{
    if (AIManager_PARAM) 
    {
        myAIManager = AIManager_PARAM;

        myAIManager->Set_Max_Entity_Count(MaxSpawnedEntities);

        OnAIManagerReady.Broadcast();
    }
}

void AGS_DungeonGeneration::Notify_AI_Manager_Patrol_Batch(FAIManagerBatchPacket BatchPacketToSend)
{
    myAIManager->Push_Patrol_Function_To_Batch(BatchPacketToSend);
}

void AGS_DungeonGeneration::UpdateOldMovementCell(FIntPoint CellInfo)
{
    //First check if the inputted cell info is within bounds to avoid crashing
    if (!(NavigationGrid.IsValidIndex(CellInfo.X)))
    {
        UE_LOG(LogTemp, Error, TEXT("The X Spawn Cells where not in range for the navigation grid"));
        return;
    }
    else if (!(NavigationGrid[CellInfo.X].IsValidIndex(CellInfo.Y)))
    {
        UE_LOG(LogTemp, Error, TEXT("The Y Spawn Cells where not in range for the navigation grid"));
        return;
    }

    //Make the old cell movable by setting the variable at the index location to true
    NavigationGrid[CellInfo.X][CellInfo.Y] = true;
}

FIntPoint AGS_DungeonGeneration::GetPlayerCoords()
{
    return CurrentPlayerCoords;
}

void AGS_DungeonGeneration::UpdatePlayerCoords(AActor* PlayerRef, FIntPoint NewCoords)
{
    //First check if the actor is what the current player controller is controlling

    if (PlayerRef == GetWorld()->GetFirstPlayerController()->GetPawn()) 
    {
        UE_LOG(LogTemp, Display, TEXT("Successfully Updated Player Coords in GS"));
        CurrentPlayerCoords = NewCoords;

    }
    else 
    {
        UE_LOG(LogTemp, Error, TEXT("Wrong actor tried to Update Player Coords in GS"));
    }
}

TArray<TArray<EDungeonGenerationType>> AGS_DungeonGeneration::Get_Dungeon_Grid_Info()
{
	return DungeonGridInfo;
}

bool AGS_DungeonGeneration::Can_Move_Forward(int StartX, int StartY, float CurrentYaw)
{
	// Check if StartX and StartY are in valid bounds
    if (!(NavigationGrid.IsValidIndex(StartX))) return false;
    if (!(NavigationGrid[StartX].IsValidIndex(StartY))) return false;

    //UE_LOG(LogTemp, Warning, TEXT("Trying to move forward current rotation is %f"), CurrentYaw);
    //UE_LOG(LogTemp, Warning, TEXT("Current X =  %d"), StartX);
    //UE_LOG(LogTemp, Warning, TEXT("Current Y =  %d"), StartY);


    // Check for "up" movement (yaw = 0)
    if (FMath::IsNearlyEqual(CurrentYaw, 0.0f, 10.f))
    {
        //Check place moving to is valid
        if (NavigationGrid[StartX].IsValidIndex(StartY - 1)) 
        {
            return NavigationGrid[StartX][StartY - 1]; // Upward
        }
    }
    // Check for "down" movement (yaw = 180)
    else if (FMath::IsNearlyEqual(CurrentYaw, 180.0f, 10.f))
    {
        //Check place moving to is valid
        if (NavigationGrid[StartX].IsValidIndex(StartY + 1)) 
        {
            return NavigationGrid[StartX][StartY + 1]; // Downward
        }
    }
    // Check for "left" movement (yaw = 270)
    else if (FMath::IsNearlyEqual(CurrentYaw, 270.0f, 10.f))
    {
        //Check place moving to is valid
        if (NavigationGrid.IsValidIndex(StartX - 1)) 
        {
            return NavigationGrid[StartX - 1][StartY]; // Leftward
        }
    }
    // Check for "right" movement (yaw = 90)
    else if (FMath::IsNearlyEqual(CurrentYaw, 90.0f, 10.f))
    {
        //Check place moving to is valid
        if (NavigationGrid.IsValidIndex(StartX + 1)) 
        {
            return NavigationGrid[StartX + 1][StartY]; // Rightward
        }
    }

    return false; // Default to false if out of bounds or any failure condition
}

void AGS_DungeonGeneration::Moving_Forward(int& StartX, int& StartY, float CurrentYaw)
{
    //I know that this function is going to be used with canmoveforward function that checks if the indices
    //are valid but I will check again in case I forget or something etc etc.
    if (!NavigationGrid.IsValidIndex(StartX)) 
    {
        UE_LOG(LogTemp, Error, TEXT("GS: Inputted startx index is not valid for navigation grid"));
        return;
    }
    if (!NavigationGrid[StartX].IsValidIndex(StartY)) 
    {
        UE_LOG(LogTemp, Error, TEXT("GS: Inputted starty index is not valid for navigation grid"));
        return;
    }

    //Since you are going to move forward, make the previous cell movable which is the StartX/Y before being
    //changed in this function
    NavigationGrid[StartX][StartY] = true;

    // Check for "up" movement (yaw = 0)
    if (FMath::IsNearlyEqual(CurrentYaw, 0.0f, 10.f))
    {
        StartY--;
    }
    // Check for "down" movement (yaw = 180)
    else if (FMath::IsNearlyEqual(CurrentYaw, 180.0f, 10.f))
    {
        StartY++;
    }
    // Check for "left" movement (yaw = 270)
    else if (FMath::IsNearlyEqual(CurrentYaw, 270.0f, 10.f))
    {
        StartX--;
    }
    // Check for "right" movement (yaw = 90)
    else if (FMath::IsNearlyEqual(CurrentYaw, 90.0f, 10.f))
    {
        StartX++;
    }

    //After moving to the new location, set that cell to not be movable as there is an entity on it
    NavigationGrid[StartX][StartY] = false;
}
