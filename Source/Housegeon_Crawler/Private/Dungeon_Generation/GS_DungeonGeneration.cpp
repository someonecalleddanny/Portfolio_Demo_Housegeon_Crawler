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

bool AGS_DungeonGeneration::Can_Move_Forward(int StartX, int StartY, float CurrentYaw)
{
	// Check if StartX and StartY are in valid bounds
    if (!(NavigationGrid.IsValidIndex(StartX))) return false;
    if (!(NavigationGrid[StartX].IsValidIndex(StartY))) return false;

    UE_LOG(LogTemp, Warning, TEXT("Trying to move forward current rotation is %f"), CurrentYaw);
    UE_LOG(LogTemp, Warning, TEXT("Current X =  %d"), StartX);
    UE_LOG(LogTemp, Warning, TEXT("Current Y =  %d"), StartY);


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
}
