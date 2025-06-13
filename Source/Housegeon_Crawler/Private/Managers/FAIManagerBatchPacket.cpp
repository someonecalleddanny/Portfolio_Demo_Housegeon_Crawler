// Fill out your copyright notice in the Description page of Project Settings.


#include "Managers/FAIManagerBatchPacket.h"

FAIManagerBatchPacket FAIManagerBatchPacket::Set_Batch_Packet(TWeakObjectPtr<APawn> ControlledPawnRef_Param, bool bIsRotating_Param, float AX, float AY, float AZ, float BX, float BY, float BZ, float TimeToFinishEvent, TFunction<void()> BindOnFinishedEvent)
{
    if (!(ControlledPawnRef_Param.IsValid()))
    {
        UE_LOG(LogTemp, Error, TEXT("Inputted pawn for set batch packet was invalid!"));
    }

    //Check if the coder accidently creates a lerp function that is 0 time, means that the batching will not be as effective
    //as it will essentially be running on tick with no delay
    if (TimeToFinishEvent <= 0.01f) 
    {
        TimeToFinish = 0.1f;
    }
    else 
    {
        TimeToFinish = TimeToFinishEvent;
    }

    ControlledPawnRef = ControlledPawnRef_Param;
    bIsRotating = bIsRotating_Param;
    FunctionWrapperOnFinished = BindOnFinishedEvent;

    myStartRotOrLoc.X = AX;
    myStartRotOrLoc.Y = AY;
    myStartRotOrLoc.Z = AZ;

    myEndRotOrLoc.X = BX;
    myEndRotOrLoc.Y = BY;
    myEndRotOrLoc.Z = BZ;

    return *(this);
}

void FAIManagerBatchPacket::Set_Alpha(float Alpha)
{
    CurrentAlpha = FMath::Clamp(Alpha, 0.f, 1.f);
}

FThreeFloatContainer FAIManagerBatchPacket::Get_Start_XYZ()
{
    return myStartRotOrLoc;
}

FThreeFloatContainer FAIManagerBatchPacket::Get_End_XYZ()
{
    return myEndRotOrLoc;
}

TWeakObjectPtr<APawn> FAIManagerBatchPacket::Get_Pawn()
{
    return ControlledPawnRef;
}

bool FAIManagerBatchPacket::Is_Rotating()
{
    return bIsRotating;
}

float FAIManagerBatchPacket::Get_Time_To_Finish()
{
    return TimeToFinish;
}

float FAIManagerBatchPacket::Get_Alpha()
{
    return CurrentAlpha;
}

void FAIManagerBatchPacket::Call_OnFinished()
{
    if (FunctionWrapperOnFinished) 
    {
        FunctionWrapperOnFinished();
    }
    else 
    {
        UE_LOG(LogTemp, Error, TEXT("Function wrapper invalid!"));
    }
}
