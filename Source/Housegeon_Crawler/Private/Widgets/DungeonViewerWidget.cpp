// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/DungeonViewerWidget.h"

void UDungeonViewerWidget::SetResetCameraVisibility(bool bIsVisible)
{
	(bIsVisible)
		? ResetCameraButton->SetVisibility(ESlateVisibility::Visible)
		: ResetCameraButton->SetVisibility(ESlateVisibility::Hidden);
}

void UDungeonViewerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ResetCameraButton->OnClicked.AddDynamic(this, &UDungeonViewerWidget::OnCameraButtonClicked);
}

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
	if (!bCanDrag && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bCanDrag = true;
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FReply UDungeonViewerWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	//Once the mouse button is up, I can stop dragging the camera with the onmousedrag function
	if (bCanDrag && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bCanDrag = false;
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FReply UDungeonViewerWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	//Get the mouse deltas
	FVector2D MouseDeltas = InMouseEvent.GetCursorDelta();
	float DeltaX = MouseDeltas.X; 
	float DeltaY = MouseDeltas.Y;

	//Check if able to drag. Set within the OnMouseDown function
	if (bCanDrag) 
	{
		//Check if both mouse deltas aren't 0.0f (Meaning that the mouse has not been moved). Added an error tolerance
		//of 0.1 because I want large mouse strokes to count as dragging.
		if (!(FMath::IsNearlyEqual(DeltaX, 0.0f, 0.1) && FMath::IsNearlyEqual(DeltaY, 0.0f, 0.1)))
		{
			//Broadcast to the DungeonViewer pawn to respond to the mouse movement
			Call_OnMouseDrag.Broadcast(DeltaX, DeltaY);

			//Finally return handled
			return FReply::Handled();
		}	
	}

	//if all the checks fail, return unhandled
	return FReply::Unhandled();
}

FReply UDungeonViewerWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	//Check if the key was already pressed because this event keeps firing when a key is pressed
	if (!bKeyAlreadyDown) 
	{
		//Set to true so that the event can't keep firing when the jey is down
		bKeyAlreadyDown = true;
		//When pressing M the widget will delete, but this will be called within the bp because I use the level bp to
		//Enable the post processing when going back to player possession
		if (InKeyEvent.GetKey() == EKeys::M)
		{
			Call_WidgetExitEvent();
		}

		//Finally return handled
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

FReply UDungeonViewerWidget::NativeOnKeyUp(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (bKeyAlreadyDown) 
	{
		bKeyAlreadyDown = false;

		//Finally return handled
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

void UDungeonViewerWidget::NativeDestruct()
{
	Super::NativeDestruct();

	//Once the widget is destroyed, destroy all delegates
	Call_OnMouseWheel.Clear();
	Call_OnMouseDrag.Clear();
	Call_OnResetCamera.Clear();
}

void UDungeonViewerWidget::OnCameraButtonClicked()
{
	//Broadcast a call to the Dungeon Viewer pawn to reset the actor's transform to the default position
	Call_OnResetCamera.Broadcast();
}
