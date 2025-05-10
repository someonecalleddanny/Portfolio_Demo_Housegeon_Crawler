// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/DungeonViewerWidget.h"

FReply UDungeonViewerWidget::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	//Get the wheel delta, if > 0 means positive which means it is going up
	if (InMouseEvent.GetWheelDelta() > 0.0f)
	{
		UE_LOG(LogTemp, Display, TEXT("Mouse Wheel Up!"));
	}
	else 
	{
		UE_LOG(LogTemp, Display, TEXT("Mouse Wheel Down!"));
	}

	Call_OnMouseWheel.Broadcast(InMouseEvent.GetWheelDelta());

	return FReply::Handled();
}
