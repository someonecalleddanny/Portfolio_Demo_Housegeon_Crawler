// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DungeonViewerWidget.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMouseWheel, float, ScrolledUpDown);

/**
 * 
 */
UCLASS()
class HOUSEGEON_CRAWLER_API UDungeonViewerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	FOnMouseWheel Call_OnMouseWheel;

protected:
	virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	
};
