// Fill out your copyright notice in the Description page of Project Settings.


#include "Dungeon_Generation/POI_Base_Class.h"

// Sets default values
APOI_Base_Class::APOI_Base_Class()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	myFloorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlayerMan"));
	myFloorMesh->SetupAttachment(RootComponent);

	Tags.Add("POI");
}

// Called when the game starts or when spawned
void APOI_Base_Class::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APOI_Base_Class::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APOI_Base_Class::Interacted()
{
	Call_Go_To_Next_Level();
}

