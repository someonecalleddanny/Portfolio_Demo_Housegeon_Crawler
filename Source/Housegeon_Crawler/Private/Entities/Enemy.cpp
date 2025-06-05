// Fill out your copyright notice in the Description page of Project Settings.


#include "Entities/Enemy.h"

// Sets default values
AEnemy::AEnemy()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//I create a scene component so that the capsule component can have a relative offset so the pivot point can be at
	//0 on the z, Don't care about losing gravity as this game will never simulate physics
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("MySceneComponent"));

	RootComponent = SceneComponent;

	myCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("PlayerMan"));

	//Attach to the rootcomponent, every other component will be a child of the capsule component
	myCapsule->SetupAttachment(RootComponent);

	myCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//Average height of the player
	myCapsule->SetCapsuleHalfHeight(88.f);
	myCapsule->SetCapsuleRadius(44.f);

	//Create the offset so that there is a normalised pivot at the bottom, crucial for when I set up my skeleton 
	myCapsule->SetRelativeLocation(FVector(0.f, 0.f, 88.f));

	EnemySkeleton = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MySkeleton"));
	EnemySkeleton->SetupAttachment(myCapsule);

	EnemySkeleton->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

