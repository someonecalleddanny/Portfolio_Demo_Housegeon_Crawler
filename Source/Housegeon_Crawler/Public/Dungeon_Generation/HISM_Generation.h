// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Dungeon_Generation/EDungeonGenerationType.h"
#include "Dungeon_Generation/GS_DungeonGeneration.h"

#include "HISM_Generation.generated.h"

UCLASS()
class HOUSEGEON_CRAWLER_API AHISM_Generation : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHISM_Generation();

	FORCEINLINE class UHierarchicalInstancedStaticMeshComponent* GetWalls() const { return HISM_Walls; }

	FORCEINLINE class UHierarchicalInstancedStaticMeshComponent* GetFloors() const { return HISM_Floors; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY()
	AGS_DungeonGeneration* myDungeonState;

	UFUNCTION()
	void Start_Generation();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Capsule, meta = (AllowPrivateAccess = "true"))
	UHierarchicalInstancedStaticMeshComponent* HISM_Walls;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Capsule, meta = (AllowPrivateAccess = "true"))
	UHierarchicalInstancedStaticMeshComponent* HISM_Floors;

	void Display_All_Walls(TArray<TArray<EDungeonGenerationType>> DungeonGridInfo_PARAM);

	FRotator Get_Random_Generation_Rotation();
};
