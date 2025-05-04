// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "Dungeon_Generation/EDungeonGenerationType.h"

#include "Housegeon_Game_Base.generated.h"

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

	void Dungeon_Logic_Finished();
};
