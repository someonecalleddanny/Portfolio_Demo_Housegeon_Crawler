// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"

#include "GameFramework/SpringArmComponent.h"

#include "Dungeon_Generation/GS_DungeonGeneration.h"

#include "DungeonViewer.generated.h"

UCLASS()
class HOUSEGEON_CRAWLER_API ADungeonViewer : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ADungeonViewer();

	FORCEINLINE class USpringArmComponent* GetCamera() const { return mySpringArm; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void Spawn_At_Center_Grid();

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Dungeon Viewing Settings");
	float DefaultLength_Multiplier = 500.f;


	UPROPERTY()
	AGS_DungeonGeneration* myDungeonState;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++ Components", meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* mySpringArm;

};
