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

#include "Camera/CameraShakeBase.h"

#include "MC.generated.h"


struct NormalizedGridTransform 
{
	int X = 0;
	int Y = 0;
	float NormalizedYaw = 0.f;
};

UCLASS()
class HOUSEGEON_CRAWLER_API AMC : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMC();

	FORCEINLINE class UCameraComponent* GetCamera() const { return myCamera; }
	FORCEINLINE UCapsuleComponent* GetMyCapsule() const { return myCapsule; }
	FORCEINLINE UTimelineComponent* GetMyTimeline() const { return MovementTimeline; }
	FORCEINLINE UTimelineComponent* GetMyRotate90Timeline() const { return Rotate90Timeline; }
	FORCEINLINE UTimelineComponent* GetMyRotate180Timeline() const { return Rotate180Timeline; }
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "C++ Properties")
	float Camera_Z = 70.f;

	UFUNCTION()
	void Spawn_At_Center_Grid();

	//Grid Based Movement Based Logic
	NormalizedGridTransform myGridTransform;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "C++ Grid Movement Logic")
	bool bKeepWalkingForward = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++ Grid Movement Logic")
	bool bAbleToMove = true;

	
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

	/*
		Might be confusing why there are 3 different move forwards, but this is the one that actually moves forward.
		Have this confusion because I want the player to keep moving when move forward key is pressed which means I need
		a function that can be used within C++ and BP for timeline lerp logic
	*/
	UFUNCTION(BlueprintCallable, Category = "C++ Functions")
	void Manual_MoveForward();

	UFUNCTION(BlueprintImplementableEvent, Category = "C++ Events")
	void Call_Move_Forward(FVector BeforeLocation, FVector DesiredLocation);

	//Blueprint Called Events
	UFUNCTION(BlueprintImplementableEvent, Category = "C++ Events")
	void Call_Rotate_90(FRotator BeforeRotation, FRotator DesiredRotation);

	UFUNCTION(BlueprintImplementableEvent, Category = "C++ Events")
	void Call_Rotate_180(FRotator BeforeRotation, FRotator DesiredRotation);

	//All the Input stuff
	UPROPERTY()
	AGS_DungeonGeneration* myDungeonState;

	UPROPERTY(EditAnywhere, Category = "C++ Inputs")
	UInputMappingContext* myMappingContext;

	UPROPERTY(EditAnywhere, Category = "C++ Inputs")
	UInputAction* IA_MoveForward;

	UPROPERTY(EditAnywhere, Category = "C++ Inputs")
	UInputAction* IA_RotateLeftRight;

	UPROPERTY(EditAnywhere, Category = "C++ Inputs")
	UInputAction* IA_RotateLeftRight180;

	UPROPERTY(EditAnywhere, Category = "C++ Inputs")
	UInputAction* IA_Interacted;

	void MoveForward(const FInputActionValue& Value);
	void RotateLeftRight(const FInputActionValue& Value);
	void Rotate180(const FInputActionValue& Value);
	void Interacted(const FInputActionValue& Value);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:

	//Player Grid Movement
	FIntPoint OldCell;

	void SetRandomSpawnRotation();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++ Components", meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* myCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Capsule, meta = (AllowPrivateAccess = "true"))
	UCapsuleComponent* myCapsule;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++ Components", meta = (AllowPrivateAccess = "true"))
	UTimelineComponent* MovementTimeline;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++ Components", meta = (AllowPrivateAccess = "true"))
	UTimelineComponent* Rotate90Timeline;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++ Components", meta = (AllowPrivateAccess = "true"))
	UTimelineComponent* Rotate180Timeline;

	//Timeline Variables
	FVector MoveForwardStartLocation;
	FVector MoveForwardEndLocation;

	FRotator TimelineStartRotation;
	FRotator TimelineEndRotation;
};
