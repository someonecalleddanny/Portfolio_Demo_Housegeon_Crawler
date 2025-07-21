// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"

#include "Dungeon_Generation/GS_DungeonGeneration.h"
#include "Dungeon_Generation/EDungeonGenerationType.h"

#include "Components/TimelineComponent.h"
#include "Curves/CurveFloat.h"

#include "Camera/CameraComponent.h" 
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"

#include "Camera/CameraShakeBase.h"

#include "Unique_ENUMS/ECompassDirection.h"

#include "Combat/FWeaponAnimationInfo.h"

#include "MC.generated.h"

enum EWeaponAnimationState
{
	AtoB,
	BtoC,
	CtoA
};

UCLASS()
class HOUSEGEON_CRAWLER_API AMC : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMC();

	//All the components made
	FORCEINLINE class UCameraComponent* GetCamera() const { return myCamera; }
	FORCEINLINE UCapsuleComponent* GetMyCapsule() const { return myCapsule; }
	FORCEINLINE UTimelineComponent* GetMyTimeline() const { return MovementTimeline; }
	FORCEINLINE UTimelineComponent* GetMyRotate90Timeline() const { return Rotate90Timeline; }
	FORCEINLINE UTimelineComponent* GetMyRotate180Timeline() const { return Rotate180Timeline; }
	FORCEINLINE UTimelineComponent* GetMyRightHandAnimationTimeline() const { return RightHandAnimationTimeline; }
	FORCEINLINE UStaticMeshComponent* GetMyRightArm() const { return RightArm; }
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "C++ Properties")
	float Camera_Z = 70.f;

	UFUNCTION()
	void Spawn_At_Center_Grid();

	//Movement timeline variables

	bool bAbleToMove = true;

	//OnFinished Timeline variables for movement
	bool bAlreadyMovingLeftRight = false;

	bool bAlreadyMovingForward = false;

	//Rotation input logic
	bool bAlreadyRotating = false;

	bool bRotate90StillHeldDown = false;

	bool bRotate90WaitingForRelease = false;

	//Forward, backward, left, right and diagonal logic
	bool bPathBlockedButPossibleDiagonal = false;

	bool bMoveForwardInputStillHeldDown = false;

	bool bMoveForwardBackwardWaitingForRelease = false;

	bool bMoveLeftRightInputStillHeldDown = false;

	bool bMoveLeftRightWaitingForRelease = false;

	//Input reader variables
	float MovementLeftRightChecker = 0.f;

	float MovementForwardBackwardChecker = 0.f;

	float RotationLeftRightChecker = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++ Grid Movement Logic")
	float DiagonalMovementAcceptanceDelay = 0.1f;

	//Camera Shakes

	UPROPERTY(EditAnywhere, Category = "C++ Camera Shakes")
	TSubclassOf<UCameraShakeBase> MoveForwardCameraShakeClass;

	//Timeline Logic

	UPROPERTY(EditAnywhere, Category = "Timeline")
	UCurveFloat* MovementFloatCurve;

	UPROPERTY(EditAnywhere, Category = "Timeline")
	float MovementForwardPlayRate = 1.25f;

	UPROPERTY(EditAnywhere, Category = "Timeline")
	float Rotate90PlayRate = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Timeline")
	float Rotate180PlayRate = 1.f;

	//Timeline event binders
	UPROPERTY()
	FOnTimelineFloat MovementInterp;

	UPROPERTY()
	FOnTimelineEvent MovementFinished;

	UPROPERTY()
	FOnTimelineFloat Rotate90Interp;

	UPROPERTY()
	FOnTimelineEvent Rotate90Finished;

	UPROPERTY()
	FOnTimelineFloat Rotate180Interp;

	UPROPERTY()
	FOnTimelineEvent Rotate180Finished;

	UPROPERTY()
	FOnTimelineFloat RightHandMeshMovementInterp;

	UPROPERTY()
	FOnTimelineEvent RightHandMeshMovementFinished;

	/*
		BOUND TIMELINE FUNCTIONS
	*/

	UFUNCTION()
	void OnMovementTimelineTick(float Alpha);

	UFUNCTION()
	void OnMovementTimelineFinished();

	UFUNCTION()
	void OnRotate90TimelineTick(float Alpha);

	UFUNCTION()
	void OnRotate90TimelineFinished();

	UFUNCTION()
	void OnRotate180TimelineTick(float Alpha);

	UFUNCTION()
	void OnRotate180TimelineFinished();

	UFUNCTION()
	void OnRightHandMeshMovementTimelineTick(float Alpha);

	UFUNCTION()
	void OnRightHandMeshMovementTimelineFinished();

	//Movement logic

	/*
		Might be confusing why there are 3 different move forwards, but this is the one that actually moves forward.
		Have this confusion because I want the player to keep moving when move forward key is pressed which means I need
		a function that can be used within C++ and BP for timeline lerp logic
	*/
	UFUNCTION(BlueprintCallable, Category = "C++ Functions")
	void Manual_MoveForward();

	void Manual_MoveLeftRight();

	void Manual_Rotate90();

	FTimerHandle TH_DelayedDiagonalFromPossibleBlockedPath;

	void Timer_DelayedBlockedDiagonalChecker();

	//All the Input stuff
	UPROPERTY()
	AGS_DungeonGeneration* myDungeonState;

	UPROPERTY(EditAnywhere, Category = "C++ Inputs")
	UInputMappingContext* myMappingContext;

	UPROPERTY(EditAnywhere, Category = "C++ Inputs")
	UInputAction* IA_MoveForwardAndBackward;

	UPROPERTY(EditAnywhere, Category = "C++ Inputs")
	UInputAction* IA_MoveLeftRight;

	UPROPERTY(EditAnywhere, Category = "C++ Inputs")
	UInputAction* IA_RotateLeftRight;

	UPROPERTY(EditAnywhere, Category = "C++ Inputs")
	UInputAction* IA_RotateLeftRight180;

	UPROPERTY(EditAnywhere, Category = "C++ Inputs")
	UInputAction* IA_Interacted;

	UPROPERTY(EditAnywhere, Category = "C++ Inputs")
	UInputAction* IA_RightAttack;

	//bound input functions

	void MoveForwardAndBackward(const FInputActionValue& Value);
	void MoveLeftRight(const FInputActionValue& Value);
	void RotateLeftRight(const FInputActionValue& Value);
	void Rotate180(const FInputActionValue& Value);
	void Interacted(const FInputActionValue& Value);
	void RightAttack(const FInputActionValue& Value);

	/*
		All the attack animation logic. As I add more weapons with different attack animations, I will move this into a
		Database, but first I need to debug all of the animations to see which looks the coolest before reading from a 
		database
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Animations")
	FTransform RightHandStartTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Animations")
	TArray<FWeaponAnimationInfo> RightHandAttackAnimations;

	UPROPERTY(EditAnywhere, Category = "Attack Animations")
	UCurveFloat* RightHandAttackCurve;

	UPROPERTY(EditAnywhere, Category = "Attack Animations")
	TSubclassOf<UCameraShakeBase> AttackShake;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:

	//Right Hand Weapon logic
	EWeaponAnimationState CurrentRightHandWeaponAnimationState;
	bool bRightHandIsAttacking = false;
	FTransform Temp_StartRightHandTransform;
	FTransform Temp_EndRightHandTransform;
	int CurrentWeaponAnimIndex = 0;
	bool bRightHandHit = false;
	bool bFullyCompletedRightHand = true;

	void Attack_One_Cell_Forward();

	//Player Grid Movement
	FIntPoint CurrentCell;
	FCompassDirection CurrentCompassDirection;

	void SetRandomSpawnRotation();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++ Components", meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* myCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Capsule, meta = (AllowPrivateAccess = "true"))
	UCapsuleComponent* myCapsule;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++ Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* RightArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++ Components", meta = (AllowPrivateAccess = "true"))
	UTimelineComponent* MovementTimeline;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++ Components", meta = (AllowPrivateAccess = "true"))
	UTimelineComponent* Rotate90Timeline;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++ Components", meta = (AllowPrivateAccess = "true"))
	UTimelineComponent* Rotate180Timeline;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++ Components", meta = (AllowPrivateAccess = "true"))
	UTimelineComponent* RightHandAnimationTimeline;

	void SetRightHandAnimationPlayRate(float InTime);

	void PlayRightHandAnimation(EWeaponAnimationState AnimationToPlay, int WeaponAnimationIndex);

	//Timeline Variables
	FVector MoveForwardStartLocation;
	FVector MoveForwardEndLocation;

	FRotator TimelineStartRotation;
	FRotator TimelineEndRotation;
};
