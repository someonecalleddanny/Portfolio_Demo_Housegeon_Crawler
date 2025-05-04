// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"

#include "Camera/CameraComponent.h" 
#include "Components/CapsuleComponent.h"

#include "MC.generated.h"

UCLASS()
class HOUSEGEON_CRAWLER_API AMC : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMC();

	FORCEINLINE class UCameraComponent* GetCamera() const { return myCamera; }
	FORCEINLINE UCapsuleComponent* GetMyCapsule() const { return myCapsule; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "C++ Properties")
	float Camera_Z = 70.f;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++ Components", meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* myCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Capsule, meta = (AllowPrivateAccess = "true"))
	UCapsuleComponent* myCapsule;

};
