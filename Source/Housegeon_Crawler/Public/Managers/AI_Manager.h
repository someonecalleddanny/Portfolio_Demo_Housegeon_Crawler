// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "AI_Manager.generated.h"

UCLASS()
class HOUSEGEON_CRAWLER_API AAI_Manager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAI_Manager();

	void Set_Max_Entity_Count(int Amount);

	void Add_Patrol_Function_To_Batch(AController* AIControllerToBatch);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//A circular queue container that will batch all the pursue events that AI controllers pass through
	UPROPERTY()
	TArray<AController*> PursueEventBatchQueueContainer;
	int PursueHead = 0;
	int PursueTail = 0;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	FTimerHandle TH_PursueBatcher;
	int MaxBatchableEntitiesInWorld = 0;

	TArray<int>Test;
	bool bTestInitted = false;
};
