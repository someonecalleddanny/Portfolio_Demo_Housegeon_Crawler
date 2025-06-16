// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FWeaponAnimationInfo.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FWeaponAnimationInfo
{
	GENERATED_BODY()

	/*
		Don't have an A transform as that is expected to be a global variable in the class, because you don't want to
		duplicate the start transform on each new attack pattern as that would just repeat the same line of code on each
		new struct created in, most likely, an attack animation array
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++ Properties")
	FTransform BTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++ Properties")
	float AtoBTime = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++ Properties")
	FTransform CTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++ Properties")
	float BtoCTime = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "C++ Properties")
	float CtoATime = 1.f;
	
};
