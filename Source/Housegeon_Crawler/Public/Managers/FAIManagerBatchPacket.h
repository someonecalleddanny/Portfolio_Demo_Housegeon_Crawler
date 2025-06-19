// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FAIManagerBatchPacket.generated.h"

/*
	This is a struct to hold 3 floats, don't use FVector because I don't want to confuse a coder when I return a location
	or rotation. I want to aggressively try and make the centralised tick function in the AI manager to be as optimised
	as possible. When I do a lerp for rotation or location, I only need 3 floats which is why I make this struct
*/
struct FThreeFloatContainer 
{
	float X = 0.f;
	float Y = 0.f;
	float Z = 0.f;
};

USTRUCT(BlueprintType)
struct FAIManagerBatchPacket 
{
	GENERATED_BODY()

public:

	FAIManagerBatchPacket Set_Batch_Packet(TWeakObjectPtr<APawn> ControlledPawnRef_Param, bool bIsRotating_Param,
		float AX, float AY, float AZ, float BX, float BY, float BZ, float TimeToFinishEvent,
		TFunction<void()> BindOnFinishedEvent);

	/*
		This function is called when you need to do AI logic as the batch packet gets popped from the ai manager queue.
		Unlike, setting the batch packet, I don't need all the other variables apart from the controlled pawn to know that
		the batch packet struct is indeed valid as it will be set and returned to ai manager from the bound function
	*/
	void Set_Delayed_Batch_Packet(TWeakObjectPtr<APawn> ControlledPawnRef_Param,
		TFunction<FAIManagerBatchPacket()> DelayBatchFunctionBind);

	FORCEINLINE void Finished_OnDelayed_Function();

	
	FORCEINLINE bool Is_A_Delayed_Batch_Packet();

	FORCEINLINE bool Check_If_Active_Batch_Packet();

	FORCEINLINE void Set_Batch_Packet_Finished();

	/*
		Once you finish the alpha within the centralised tick, set the new alpha for the next tick, this function clamps
		from 0 to 1 so don't worry for any potential game breaking bugs
	*/
	FORCEINLINE void Set_Alpha(float Alpha);

	/*
		Have FORCEINLINE functions to cache the getters from the batch packet struct to ensure as much efficiency 
		as possible within the centralised tick function within the ai manager
	*/
	FORCEINLINE FThreeFloatContainer Get_Start_XYZ();
	FORCEINLINE FThreeFloatContainer Get_End_XYZ();
	FORCEINLINE TWeakObjectPtr<APawn> Get_Pawn();
	FORCEINLINE bool Is_Rotating();
	FORCEINLINE float Get_Time_To_Finish();
	FORCEINLINE float Get_Alpha();

	//Don't need to inline as not called every frame but once when the lerp has finished
	void Call_OnFinished();

	//Call the delayed function to replace the current batch packet to the batch that needs to be executed when popped
	FAIManagerBatchPacket Call_OnDelayedFunction();
private:
	//So, the enemy can either be rotating or moving to another cell, Instead of making a start/end FVector and FRotator
	//I thought to create my own struct with 3 floats as the enemy will either be rotating or moving on a lerp which will
	//use 3 floats anyway
	FThreeFloatContainer myStartRotOrLoc;
	FThreeFloatContainer myEndRotOrLoc;

	//The bool determines if the enemy is going to be rotating or moving to another cell in the ai manager tick
	bool bIsRotating = false;

	//This is the lerp logic that will be used to execute the tasks defined by the ai controller
	float TimeToFinish = 0.f;
	float CurrentAlpha = 0.f;

	//Need the controlled pawn from the ai controller to allow the manager to move the enemy
	TWeakObjectPtr<APawn> ControlledPawnRef;

	/*
		So I decided to test and use functors instead of an interface. At time on implementation I didn't really know
		speed benefits of which is fastest: function pointer -> interface -> Funtor. This is because interface functions
		are stored within a look up table. However, I still have the problem of circular dependency that really needs to be
		rooted out before I can start using interface function. Yes I could forward declare but I would have to cast the
		functions within my ai manager anyway (maybe might not have much of a difference compared to functor). So, I need
		to find a way to either store the interface so I can just call it right out the gate and not cast or learn more
		about how I can do function pointers because I need to optimise this further for mid/low systems. My work rig
		is fine but more work needs to be done. At time of writing this is good enough for a showcase as this will take
		around 1-2 weeks max, to refactor.
	*/

	// Function to call when finished (bind this from my AIController), Use this because I cannot include the ai controller
	//as a header within my ai manager as a circular dependency will happen as the GS owns the ai manager and the 
	TFunction<void()> FunctionWrapperOnFinished;

	TFunction<FAIManagerBatchPacket()> FunctionWrapperDelayedAIBatch;

	bool bActiveBatch = false;
};
