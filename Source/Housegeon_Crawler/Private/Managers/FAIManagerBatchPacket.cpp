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

void FAIManagerBatchPacket::Set_Delayed_Batch_Packet(TWeakObjectPtr<APawn> ControlledPawnRef_Param, TFunction<FAIManagerBatchPacket()> DelayBatchFunctionBind)
{
    if (!DelayBatchFunctionBind) 
    {
        UE_LOG(LogTemp, Error, TEXT("The bound delayed batch function failed to bind within the Batch Packet struct"));
        return;
    }
    if (!ControlledPawnRef_Param.IsValid()) 
    {
        UE_LOG(LogTemp, Error, TEXT("The Controlled Pawn was null for when setting delayed batch packet in batch struct!"));
        return;
    }

    ControlledPawnRef = ControlledPawnRef_Param;
    FunctionWrapperDelayedAIBatch = DelayBatchFunctionBind;
}

bool FAIManagerBatchPacket::Is_A_Delayed_Batch_Packet()
{
    return bDelayedAIBatch;
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
        UE_LOG(LogTemp, Error, TEXT("On finished wrapper invalid within created struct!"));
    }
}

//Looks confusing, but you are essentially overwriting the current struct to a new batch struct that needs logic to be done
//right away rather than waiting in the queue to execute
FAIManagerBatchPacket FAIManagerBatchPacket::Call_OnDelayedFunction()
{
    if (FunctionWrapperDelayedAIBatch)
    {
        //So the function wrapper should do its logic and return the new updated batch packet to be done on the first
        //frame of the centralised tick to execute
        return FunctionWrapperDelayedAIBatch();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("On delayed wrapper invalid within created struct!"));
        //To not crash anything, just return the current batched item, will cause logical errors within the ai manager but
        //will still run
        return *(this);
    }
}
