// Fill out your copyright notice in the Description page of Project Settings.
#include "Dungeon_Generation/GS_DungeonGeneration.h"
#include "Interfaces/EnemyPawnComms.h"



void AGS_DungeonGeneration::Set_Dungeon_Grid_Info(TArray<TArray<EDungeonGenerationType>> DungeonGridInfo_Param, TArray<TArray<bool>> NavigationGrid_Param)
{
	DungeonGridInfo = DungeonGridInfo_Param;
    NavigationGrid = NavigationGrid_Param;

    EntityCoords.Empty();

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

void AGS_DungeonGeneration::Register_Entity_Cell_Location(FIntPoint EntityCellInfo, AActor* EntityToRegister)
{
    //First check if the inputted cell info is within bounds to avoid crashing
    if (!(NavigationGrid.IsValidIndex(EntityCellInfo.X)))
    {
        UE_LOG(LogTemp, Error, TEXT("The X Spawn Cells where not in range for the navigation grid"));
        UE_LOG(LogTemp, Error, TEXT("The X Input = %d"), EntityCellInfo.X);
        UE_LOG(LogTemp, Error, TEXT("Current Navigation Grid Size = %d"), NavigationGrid.Num());
        return;
    }
    else if (!(NavigationGrid[EntityCellInfo.X].IsValidIndex(EntityCellInfo.Y)))
    {
        UE_LOG(LogTemp, Error, TEXT("The Y Spawn Cells where not in range for the navigation grid"));
        return;
    }

    UE_LOG(LogTemp, Display, TEXT("Registered Entity's cell location!"));

    //Make the area not movable to any other entities
    NavigationGrid[EntityCellInfo.X][EntityCellInfo.Y] = false;

    //Then I need to register the coords for the damage system

    if (EntityCoords.Contains(EntityCellInfo)) 
    {
        EntityCoords.Remove(EntityCellInfo);
    }

    EntityCoords.Add(EntityCellInfo, EntityToRegister);
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
    if (myAIManager) 
    {
        myAIManager->Push_Patrol_Function_To_Batch(BatchPacketToSend);
    }
    else 
    {
        UE_LOG(LogTemp, Error, TEXT("AI MANAGER NOT INITTED PROPERLY!"));
    }
    
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

bool AGS_DungeonGeneration::Can_Move_Forward(int StartX, int StartY, FCompassDirection CurrentDirection)
{
    FCompassDirection::ECompassDirection CurrentDirectionENUM = CurrentDirection.Get_Current_Compass_Direction();
    switch (CurrentDirectionENUM)
    {
    case FCompassDirection::ECompassDirection::North:
        StartY--;
        break;

    case FCompassDirection::ECompassDirection::South:
        StartY++;
        break;

    case FCompassDirection::ECompassDirection::West:
        StartX--;
        break;

    case FCompassDirection::ECompassDirection::East:
        StartX++;
        break;

    default:
        UE_LOG(LogTemp, Error, TEXT("GS: current direction checked is stuck on a diagonal compass direction, fix code!"));
        return false;
    }

    if (!Check_Valid_Movement_Indices(StartX, StartY)) return false;

    //Return true from the array if there is a valid movement cell that you are going to move forward to
    return NavigationGrid[StartX][StartY];
}

void AGS_DungeonGeneration::Moving_Forward(AActor* EntityMoved, int& StartX, int& StartY, FCompassDirection CurrentDirection)
{
    if (!Check_Valid_Movement_Indices(StartX, StartY)) return;

    //Create a temp IntPoint variable for checking maps
    FIntPoint CurrentXY(StartX, StartY);
    bool bIsMovingEntityNotPlayer = false;

    //If the entity coords array contains the currentXY, then I know that it is not the player as the player has its own
    //FIntPoint variable
    if (EntityCoords.Contains(CurrentXY))
    {
        //Remove the key from the map as that is being moved from
        EntityCoords.Remove(CurrentXY);
        bIsMovingEntityNotPlayer = true;
    }

    //Since you are going to move forward, make the previous cell movable which is the StartX/Y before being
    //changed in this function
    NavigationGrid[StartX][StartY] = true;

    FCompassDirection::ECompassDirection CurrentDirectionENUM = CurrentDirection.Get_Current_Compass_Direction();

    switch (CurrentDirectionENUM)
    {
    case  FCompassDirection::ECompassDirection::North:
        StartY--;
        break;

    case  FCompassDirection::ECompassDirection::South:
        StartY++;
        break;

    case  FCompassDirection::ECompassDirection::West:
        StartX--;
        break;

    case  FCompassDirection::ECompassDirection::East:
        StartX++;
        break;

    default:
        UE_LOG(LogTemp, Error, TEXT("GS: current direction checked is stuck on a diagonal compass direction, fix code!"));
        return;
    }

    //Check the new updated StartX/Y again to see if still in bounds
    if (!Check_Valid_Movement_Indices(StartX, StartY)) return;

    //After moving to the new location, set that cell to not be movable as there is an entity on it
    NavigationGrid[StartX][StartY] = false;

    //Overwrite the currentXY with the new coords from movement
    CurrentXY = FIntPoint(StartX, StartY);

    //Check if moved the player or not to determine which hurt square/cell to update
    if (bIsMovingEntityNotPlayer)
    {
        //UE_LOG(LogTemp, Display, TEXT("Moving an entity not the player within move forward GS"));
        EntityCoords.Add(CurrentXY, EntityMoved);
    }
    else 
    {
        CurrentPlayerCoords = CurrentXY;
        //UE_LOG(LogTemp, Display, TEXT("Moved the player and updated current coords within GS"));
    }
}

bool AGS_DungeonGeneration::Check_Valid_Movement_Indices(int StartX, int StartY)
{
    if (!NavigationGrid.IsValidIndex(StartX))
    {
        UE_LOG(LogTemp, Error, TEXT("GS: Inputted startx index is not valid for navigation grid"));
        return false;
    }
    if (!NavigationGrid[StartX].IsValidIndex(StartY))
    {
        UE_LOG(LogTemp, Error, TEXT("GS: Inputted starty index is not valid for navigation grid"));
        return false;
    }

    return true;
}

void AGS_DungeonGeneration::Try_Sending_Damage_To_Entity(TArray<FIntPoint> DamageCells, float Damage)
{
    //Check each damage cell that (most likely) the player inputs from their attack, every attack is an aoe in this game
    //even if it is one cell in front, so I want to reuse this function for new attack patterns that I will create in the
    //future.
    for (FIntPoint DamageCell : DamageCells) 
    {    
        // Find returns nullptr if not found, so this alone covers Contains
        TWeakObjectPtr<AActor>* DamageEntity = EntityCoords.Find(DamageCell);

        //Check if null ptr and check if the enemy/entity is pending kill or killed etc (any dangling pointer)
        if (!DamageEntity || !DamageEntity->IsValid())
            continue;

        //Dereference the pointer
        AActor* DePointeredDamageEntity = DamageEntity->Get();

        //Check if null once again
        if (!DePointeredDamageEntity)
            continue;

        UE_LOG(LogTemp, Display, TEXT("Sending damage to enemy!"));

        IEnemyPawnComms* PawnComms = Cast<IEnemyPawnComms>(DePointeredDamageEntity);

        if (!PawnComms)
            continue;

        PawnComms->Send_Damage(Damage);
    }
}

void AGS_DungeonGeneration::Killed_An_Entity(FIntPoint CellLocation)
{

    //I know that this function is going to be used with canmoveforward function that checks if the indices
    //are valid but I will check again in case I forget or something etc etc.
    if (!NavigationGrid.IsValidIndex(CellLocation.X))
    {
        UE_LOG(LogTemp, Error, TEXT("GS: You did bad maths for the x location of your killed enemy"));
        return;
    }
    if (!NavigationGrid[CellLocation.X].IsValidIndex(CellLocation.Y))
    {
        UE_LOG(LogTemp, Error, TEXT("GS: You did bad maths for the y location of your killed enemy"));
        return;
    }

    //Make the cell location movable now
    NavigationGrid[CellLocation.X][CellLocation.Y] = true;
    //Then remove the cell location for damage squares if the AI is within it on death
    if (EntityCoords.Contains(CellLocation))
    {
        EntityCoords.Remove(CellLocation);
    }
}
