// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


struct FAIManagerBatchPacket 
{
	//These are the start locations/rotations that will be used for the lerp. I used 6 seperate floats instead of using
	//... start/end location/rotation because that would make an increased overhead as well as having more floats in general
	float StartX = 0.f;
	float StartY = 0.f;
	float StartZ = 0.f;

	float EndX = 0.f;
	float EndY = 0.f;
	float EndZ = 0.f;

	//The bool determines if the enemy is going to be rotating or moving to another cell in the ai manager tick
	bool bIsRotating = false;

	//This is the lerp logic that will be used to execute the tasks defined by the ai controller
	float TimeToFinish = 0.f;
	float CurrentAlpha = 0.f;

	//Need the controlled pawn from the ai controller to allow the manager to move the enemy
	TWeakObjectPtr<APawn> ControlledPawnRef;

	// Function to call when finished (bind this from my AIController), Use this because I cannot include the ai controller
	//as a header within my ai manager as a circular dependency will happen as the GS owns the ai manager and the 
	TFunction<void()> OnFinished;
};
