// Fill out your copyright notice in the Description page of Project Settings.


#include "Unique_ENUMS/ECompassDirection.h"

void FCompassDirection::Rotate_By_X_Amount(float RotationAdder)
{
	//Cast the compass direction which is an enum, into an int so that I can rotate the compass direction below
	//90 degrees counts as a value of 2, 45 is 1 (Have not thought of a mathematical way to scale if enum
	//...size changes so I made this function here so I can localise the issue instead of going through
	//every entity in the game that rotates) -> Will change if I find a way but this works currently
	int8 IntCastedDirection = static_cast<int8>(CompassDirection);

	//Normalise the adder to the enum int value
	int ENUM_IntCastAdder = FMath::RoundToInt((Compass_ENUM_Size) * (RotationAdder / 360.f));

	//This maths wraps things around if the value goes negative or too large from the compass size
	//Seems weird but if positive, the value will remain the same and then get modulused, if negative, make positive
	//and then modulus again
	IntCastedDirection = ((IntCastedDirection + ENUM_IntCastAdder) % Compass_ENUM_Size + Compass_ENUM_Size)
		% Compass_ENUM_Size;

	// The int value is still not updated to the enum, so cast it to update the new rotation
	CompassDirection = static_cast<ECompassDirection>(IntCastedDirection);
}
