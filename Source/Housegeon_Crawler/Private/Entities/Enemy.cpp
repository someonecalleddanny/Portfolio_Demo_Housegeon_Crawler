// Fill out your copyright notice in the Description page of Project Settings.


#include "Entities/Enemy.h"
#include "Dungeon_Generation/GS_DungeonGeneration.h"
#include "Interfaces/EnemyPawnToControllerComms.h"


// Sets default values
AEnemy::AEnemy()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

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

void AEnemy::Send_Damage(float Damage)
{
	UE_LOG(LogTemp, Display, TEXT("In enemy, interface worked, damaging: %s"), *this->GetName());

	Health -= Damage;

	if (Health <= 0.f) 
	{
		UE_LOG(LogTemp, Warning, TEXT("You have killed this enemy!"));

		//Before, destroying, I need to update the current cell to be empty so other entities can move there
		AGS_DungeonGeneration* GS = GetWorld()->GetGameState<AGS_DungeonGeneration>();
		IEnemyPawnToControllerComms* ControllerComms = Cast<IEnemyPawnToControllerComms>(GetController());

		if (GS && ControllerComms) 
		{
			GS->Killed_An_Entity(ControllerComms->GetCurrentXY());
		}

		this->Destroy();
	}
}

