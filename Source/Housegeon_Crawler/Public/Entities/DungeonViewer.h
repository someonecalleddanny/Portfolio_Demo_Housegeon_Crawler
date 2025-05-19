// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"

#include "GameFramework/SpringArmComponent.h"

#include "Dungeon_Generation/GS_DungeonGeneration.h"

#include "Widgets/DungeonViewerWidget.h"

#include "DungeonViewer.generated.h"

UCLASS()
class HOUSEGEON_CRAWLER_API ADungeonViewer : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ADungeonViewer();

	FORCEINLINE class USpringArmComponent* GetCamera() const { return mySpringArm; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PossessedBy(AController* NewController) override;

	UFUNCTION()
	void Spawn_At_Center_Grid();

	UFUNCTION()
	void ZoomInOut(float MouseWheelDelta);

	UFUNCTION()
	void RotateCameraFromWidget(float DeltaX, float DeltaY);

	UFUNCTION(BlueprintCallable, Category = "C++ Functions")
	void Reset_Camera();

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Dungeon Viewing Settings");
	float DefaultLength_Multiplier = 500.f;

	UPROPERTY()
	float CurrentCameraLength = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Viewing Settings");
	float Camera_DefaultLength = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Viewing Settings");
	float Camera_ZoomMultiplier = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Viewing Settings");
	float Min_DefaultLength = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Viewing Settings");
	float Max_DefaultLength = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Viewing Settings");
	float CameraYawSensitivity = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Viewing Settings");
	float CameraPitchSensitivity = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Viewing Settings");
	TSubclassOf<UDungeonViewerWidget> DungeonViewerWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Viewing Settings");
	UDungeonViewerWidget* DungeonViewerWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Viewing Settings");
	float Reset_Yaw = -90.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Viewing Settings");
	float Reset_Pitch = -40.f;

	UPROPERTY()
	AGS_DungeonGeneration* myDungeonState;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++ Components", meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* mySpringArm;

};
