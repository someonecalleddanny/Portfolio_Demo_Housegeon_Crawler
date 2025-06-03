// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/DungeonViewerWidget.h"

#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"

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
	//Forward delcare the mouse deltas, Do this because the mouse can wrap when reaching the leftmost and rightmost areas
	//Of the screen
	float DeltaX;
	float DeltaY;

	//Check if the player controller is valid
	APlayerController* PC = GetWorld()->GetFirstPlayerController();

	if (!PC) return FReply::Unhandled();

	//Get the mouse deltas
	FVector2D MouseDeltas = InMouseEvent.GetCursorDelta();

	//Now check if the mouse has been wrapped, Do this because setting the mouse location to the opposite x creates a large
	//Mouse delta (Mouse deltas incorporate mouse acceleration), So I get the pre wrap deltas, before they were changed
	//On the wrap
	if (bMouseWrapped)
	{
		DeltaX = PreWrapDeltaX;
		DeltaY = PreWrapDeltaY;
		bMouseWrapped = false;
	}
	else 
	{
		//Clamp the mouse deltas so that the mouse acceleration does not become too high
		MouseDeltas.X = FMath::Clamp(MouseDeltas.X, -50.f, 50.f);
		MouseDeltas.Y = FMath::Clamp(MouseDeltas.Y, -50.f, 50.f);

		DeltaX = MouseDeltas.X;
		DeltaY = MouseDeltas.Y;

		//Set the pre wrap deltas before setting the mouse location when wrapping
		PreWrapDeltaX = DeltaX;
		PreWrapDeltaY = DeltaY;
	}

	//Get the absolute mouse pos, Used when setting the new mouse location on wrap
	FVector2D AbsolutePos;
	PC->GetMousePosition(AbsolutePos.X, AbsolutePos.Y);

	//Check if able to drag. Set within the OnMouseDown function
	if (bCanDrag) 
	{
		FVector2D MousePos = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetWorld());

		//Get the size of the viewport to check when wrapping on the rightmost screen, used to set the absolute coords
		//of mouse
		FVector2D ViewportSize;
		if (GEngine && GEngine->GameViewport)
		{
			GEngine->GameViewport->GetViewportSize(ViewportSize);
		}

		//Get the widget size, This is used to compare the relative mouse position to the max size of the widget rather than
		//full size of monitor screen as the game viewport can be smaller than the total size of the monitor screen
		FVector2D WidgetSize = InGeometry.GetLocalSize();

		if (FMath::IsNearlyEqual(MousePos.X, 0.0f, 2.5f)) 
		{
			PC->SetMouseLocation(ViewportSize.X - 5.f, AbsolutePos.Y);

			//bCanDrag = false;
			bMouseWrapped = true;
			//return FReply::Handled();
		}
		else if (FMath::IsNearlyEqual(MousePos.X, WidgetSize.X, 2.5f))
		{
			PC->SetMouseLocation(0.0f + 5.f, AbsolutePos.Y);

			//bCanDrag = false;
			bMouseWrapped = true;
			//return FReply::Handled();
		}

		//Check if both mouse deltas aren't 0.0f (Meaning that the mouse has not been moved). Added an error tolerance
		//of 0.1 because I want large mouse strokes to count as dragging.
		if (!(FMath::IsNearlyEqual(DeltaX, 0.0f, 0.1) && FMath::IsNearlyEqual(DeltaY, 0.0f, 0.1)))
		{
			//Broadcast to the DungeonViewer pawn to respond to the mouse movement
			Call_OnMouseDrag.Broadcast(DeltaX, DeltaY);

			UE_LOG(LogTemp, Display, TEXT("DeltaX = %f"), DeltaX);

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
