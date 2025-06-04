// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"

#include "Enemy.generated.h"

UCLASS()
class HOUSEGEON_CRAWLER_API AEnemy : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AEnemy();

	FORCEINLINE USkeletalMeshComponent* GetMySkeleton() const { return EnemySkeleton; }
	FORCEINLINE UCapsuleComponent* GetMyCapsule() const { return myCapsule; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++ Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* EnemySkeleton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Capsule, meta = (AllowPrivateAccess = "true"))
	UCapsuleComponent* myCapsule;
};
