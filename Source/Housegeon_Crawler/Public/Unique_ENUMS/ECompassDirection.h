// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

struct FCompassDirection 
{
	enum class ECompassDirection : uint8
	{
		North,
		NorthEast,
		East,
		SouthEast,
		South,
		SouthWest,
		West,
		NorthWest,
		//Use this instead of a manual getter because this, technically, auto sizes if I put more directions behind this
		Compass_Size_For_Modulus_Wrapping_Rotation
	};

	/*
		Have a global size for the compass when adding/subtracting rotation for anything, made static to optimise memory
		for each new instance of this struct that is created to only have one held within memory. (Writing obvious
		code comment because first time using costexpr).
	*/
	static constexpr int Get_Compass_ENUM_Size()
	{
		return Compass_ENUM_Size;
	}

	ECompassDirection Get_Current_Compass_Direction() 
	{
		return CompassDirection;
	}

	void Set_Compass_Direction(ECompassDirection NewCompassDirection) 
	{
		CompassDirection = NewCompassDirection;
	}

	/*
		Rotate the compass direction either left or right, true = right, false = left
	*/
	void Rotate_90_Degrees(bool bIsGoingRight);

	void Rotate_180_Degrees();

private:
	ECompassDirection CompassDirection;
	static const int Compass_ENUM_Size = static_cast<int>(ECompassDirection::Compass_Size_For_Modulus_Wrapping_Rotation);
};
