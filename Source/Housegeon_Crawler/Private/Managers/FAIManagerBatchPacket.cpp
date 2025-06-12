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

    StartX = AX;
    StartY = AY;
    StartZ = AZ;

    EndX = BX;
    EndY = BY;
    EndZ = BZ;

    return *(this);
}
