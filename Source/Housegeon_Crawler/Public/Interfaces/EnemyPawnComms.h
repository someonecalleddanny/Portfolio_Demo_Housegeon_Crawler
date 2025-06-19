// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EnemyPawnComms.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UEnemyPawnComms : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HOUSEGEON_CRAWLER_API IEnemyPawnComms
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	virtual void Send_Damage(float Damage) = 0;
};
