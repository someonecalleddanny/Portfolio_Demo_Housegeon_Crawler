// Fill out your copyright notice in the Description page of Project Settings.


#include "Managers/AI_Manager.h"

// Sets default values
AAI_Manager::AAI_Manager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AAI_Manager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAI_Manager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

