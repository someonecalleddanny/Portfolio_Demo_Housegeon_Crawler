// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "Components/Button.h"

#include "DungeonViewerWidget.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMouseWheel, float, ScrolledUpDown);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMouseDrag, float, DeltaX, float, DeltaY);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCameraButtonClicked);

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
	FOnCameraButtonClicked Call_OnResetCamera;

protected:

	virtual void NativeConstruct() override;

	virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void NativeDestruct() override;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UButton* ResetCameraButton;

	UFUNCTION()
	void OnCameraButtonClicked();
	
private:
	bool bCanDrag = false;
};
