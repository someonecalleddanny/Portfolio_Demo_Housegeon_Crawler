// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"

#include "Interfaces/EnemyPawnComms.h"

#include "Enemy.generated.h"

UCLASS()
class HOUSEGEON_CRAWLER_API AEnemy : public APawn, public IEnemyPawnComms
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AEnemy();

	FORCEINLINE USkeletalMeshComponent* GetMySkeleton() const { return EnemySkeleton; }
	FORCEINLINE UCapsuleComponent* GetMyCapsule() const { return myCapsule; }
	FORCEINLINE USceneComponent* GetMySceneComponent() const { return SceneComponent; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Info")
	float Health = 5.f;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	//Interface function calls
	virtual void Send_Damage(float Damage) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++ Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* EnemySkeleton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Capsule, meta = (AllowPrivateAccess = "true"))
	UCapsuleComponent* myCapsule;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++ Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* SceneComponent;
};
