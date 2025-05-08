// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Components/StaticMeshComponent.h"
#include "Interfaces/POI_Interaction.h"

#include "POI_Base_Class.generated.h"

UCLASS()
class HOUSEGEON_CRAWLER_API APOI_Base_Class : public AActor, public IPOI_Interaction
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APOI_Base_Class();

	FORCEINLINE class UStaticMeshComponent* GetFloorMesh() const { return myFloorMesh; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "C++ Events")
	void Call_Go_To_Next_Level();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "C++ Components", meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* myFloorMesh;

	virtual void Interacted() override;
};
