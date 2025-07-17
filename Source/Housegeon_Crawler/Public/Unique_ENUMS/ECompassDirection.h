// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

struct FCompassDirection 
{
	enum class ECompassDirection : uint8
	{
		/*
			IF ADDING OR REMOVING NEW DIRECTIONS, MAKE UNIFORM!
		*/
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

	void Rotate_By_X_Amount(float RotationAdder);

	ECompassDirection Get_Possible_Compass_Direction_From_Added_X_Rotation(float RotationAdder)
	{
		int8 IntCastedDirection = static_cast<int8>(CompassDirection);

		//cast the adder and normalise for the compass direction ENUM
		int ENUM_IntCastAdder = FMath::RoundToInt((Compass_ENUM_Size) * (RotationAdder / 360.f));

		//Modulus the adder so that it wraps for the total size of the ECompassDirection ENUM
		IntCastedDirection = ((IntCastedDirection + ENUM_IntCastAdder) % Compass_ENUM_Size + Compass_ENUM_Size)
			% Compass_ENUM_Size;

		//Not overwriting my CompassDirection here in this function, just sending back a possible direction that could be
		//made
		ECompassDirection CompassReturn = static_cast<ECompassDirection>(IntCastedDirection);

		return CompassReturn;
	};

private:
	ECompassDirection CompassDirection;
	static const int Compass_ENUM_Size = static_cast<int>(ECompassDirection::Compass_Size_For_Modulus_Wrapping_Rotation);
};
