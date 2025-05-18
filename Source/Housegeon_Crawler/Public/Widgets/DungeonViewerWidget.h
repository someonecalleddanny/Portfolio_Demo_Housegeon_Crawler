// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DungeonViewerWidget.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMouseWheel, float, ScrolledUpDown);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMouseDrag, float, DeltaX, float, DeltaY);

/**
 * 
 */
UCLASS()
class HOUSEGEON_CRAWLER_API UDungeonViewerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	FOnMouseWheel Call_OnMouseWheel;
	FOnMouseDrag Call_OnMouseDrag;

protected:
	virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void NativeDestruct() override;
	
private:
	bool bCanDrag = false;
};
