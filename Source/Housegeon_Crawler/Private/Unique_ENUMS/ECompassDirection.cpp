// Fill out your copyright notice in the Description page of Project Settings.


#include "Unique_ENUMS/ECompassDirection.h"

void FCompassDirection::Rotate_90_Degrees(bool bIsGoingRight)
{
	//Cast the compass direction which is an enum, into an int so that I can rotate the compass direction below
	//90 degrees counts as a value of 2, 45 is 1 (Have not thought of a mathematical way to scale if enum
	//...size changes so I made this function here so I can localise the issue instead of going through
	//every entity in the game that rotates) -> Will change if I find a way but this works currently
	int8 IntCastedDirection = static_cast<int8>(CompassDirection);

	int8 RotationAdder = 2;
	if (!bIsGoingRight) RotationAdder *= -1;

	//This maths wraps things around if the value goes negative or too large from the compass size
	//Seems weird but if positive, the value will remain the same and then get modulused, if negative, make positive
	//and then modulus again
	IntCastedDirection = ((IntCastedDirection + RotationAdder) % Compass_ENUM_Size + Compass_ENUM_Size)
		% Compass_ENUM_Size;

	// The int value is still not updated to the enum, so cast it to update the new rotation
	CompassDirection = static_cast<ECompassDirection>(IntCastedDirection);
}

void FCompassDirection::Rotate_180_Degrees()
{
	//cast the enum into an int
	int8 IntCastedDirection = static_cast<int8>(CompassDirection);

	// You are mirroring a 360 degree circle so get the size and divide by two, does not matter if the adder is negative
	//or positive, the only requirement is that my ENUM is evenly spaced - SO DON'T BE A DUMBO IN THE FUTURE!!!
	int8 RotationAdder = Compass_ENUM_Size / 2;

	//Don't wrap for negatives because will never be negative unless I do something horrific in my coding adventure
	IntCastedDirection = (IntCastedDirection + RotationAdder) % Compass_ENUM_Size;

	//cast back into enum from newly rotated value
	CompassDirection = static_cast<ECompassDirection>(IntCastedDirection);
}
