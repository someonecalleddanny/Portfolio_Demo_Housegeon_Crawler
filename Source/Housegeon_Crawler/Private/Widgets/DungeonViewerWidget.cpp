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

FReply UDungeonViewerWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	//Once the mouse button is down, I can drag the camera with the onmousedrag function
	if (!bCanDrag)
	{
		bCanDrag = true;
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FReply UDungeonViewerWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	//Once the mouse button is up, I can stop dragging the camera with the onmousedrag function
	if (bCanDrag)
	{
		bCanDrag = false;
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FReply UDungeonViewerWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bCanDrag) 
	{
		FVector2D MouseDeltas = InMouseEvent.GetCursorDelta();

		float DeltaX = MouseDeltas.X;
		float DeltaY = MouseDeltas.Y;

		Call_OnMouseDrag.Broadcast(DeltaX, DeltaY);

		return FReply::Handled();
	}
	return FReply::Unhandled();
}
