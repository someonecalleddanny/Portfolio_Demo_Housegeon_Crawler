// Fill out your copyright notice in the Description page of Project Settings.


#include "Entities/MC.h"
#include "Interfaces/POI_Interaction.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AMC::AMC()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	myCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("PlayerMan"));
	myCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RootComponent = myCapsule;

	myCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraView"));
	myCamera->SetupAttachment(myCapsule);

	myCamera->SetRelativeLocation(FVector(0.f, 0.f, Camera_Z));

	RightArm = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("myRightArm"));
	RightArm->SetupAttachment(myCapsule);
	RightArm->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	MovementTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("MoveForwardTimelineComponent"));
	Rotate90Timeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("Rotate90TimelineComponent"));
	Rotate180Timeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("Rotate180TimelineComponent"));
	RightHandAnimationTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("MyRHAnimationComponent"));
	Tags.Add("MC");

	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

// Called when the game starts or when spawned
void AMC::BeginPlay()
{
	Super::BeginPlay();

	//Get my dungeon state
	myDungeonState = GetWorld()->GetGameState<AGS_DungeonGeneration>();

	//Check if valid and I was not stupid to add it to the game mode base
	if (myDungeonState)
	{
		//I found that starting generation in begin play caused glitches because the game state initted later than
		//This actor, So I have a delegate within my dungeonstate that broadcasts once the 2d array is valid to be read
		//from
		myDungeonState->OnGridReady.AddDynamic(this, &AMC::Spawn_At_Center_Grid);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("NOOOT HISM GAME STATE FOUND TO DISPLAY DUNGEON GENERATION!"));
	}

	if (APlayerController* myController = Cast<APlayerController>(GetController()))
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(myController->GetLocalPlayer());

		Subsystem->AddMappingContext(myMappingContext, 0);
	}

	if (MovementTimeline && Rotate90Timeline && Rotate180Timeline && RightHandAnimationTimeline)
	{
		//Add the move forward timeline logic, Called when W key is pressed
		MovementInterp.BindUFunction(this, FName("OnMovementTimelineTick"));
		MovementFinished.BindUFunction(this, FName("OnMovementTimelineFinished"));

		MovementTimeline->AddInterpFloat(MovementFloatCurve, MovementInterp);
		MovementTimeline->SetTimelineFinishedFunc(MovementFinished);

		MovementTimeline->SetPlayRate(MovementForwardPlayRate);

		//Add the Rotate90 timeline logic, Called when A/D key is pressed
		Rotate90Interp.BindUFunction(this, FName("OnRotate90TimelineTick"));
		Rotate90Finished.BindUFunction(this, FName("OnRotate90TimelineFinished"));

		Rotate90Timeline->AddInterpFloat(MovementFloatCurve, Rotate90Interp);
		Rotate90Timeline->SetTimelineFinishedFunc(Rotate90Finished);

		Rotate90Timeline->SetPlayRate(Rotate90PlayRate);

		//Do the same when Rotating 180 degrees when pressing S

		Rotate180Interp.BindUFunction(this, FName("OnRotate180TimelineTick"));
		Rotate180Finished.BindUFunction(this, FName("OnRotate180TimelineFinished"));

		Rotate180Timeline->AddInterpFloat(MovementFloatCurve, Rotate180Interp);
		Rotate180Timeline->SetTimelineFinishedFunc(Rotate180Finished);

		Rotate180Timeline->SetPlayRate(Rotate180PlayRate);

		//Do the same for Right Hand Animations
		RightHandMeshMovementInterp.BindUFunction(this, FName("OnRightHandMeshMovementTimelineTick"));
		RightHandMeshMovementFinished.BindUFunction(this, FName("OnRightHandMeshMovementTimelineFinished"));

		RightHandAnimationTimeline->AddInterpFloat(RightHandAttackCurve, RightHandMeshMovementInterp);
		RightHandAnimationTimeline->SetTimelineFinishedFunc(RightHandMeshMovementFinished);

		RightHandAnimationTimeline->SetPlayRate(1.f);


	}
	else 
	{
		UE_LOG(LogTemp, Error, TEXT("Timelines Not Valid"));
	}
	
}

void AMC::Spawn_At_Center_Grid()
{
	//Get The dungeon state
	TArray<TArray<EDungeonGenerationType>> DungeonGridInfo = myDungeonState->Get_Dungeon_Grid_Info();

	//Check if the array is valid so no crashes
	if (DungeonGridInfo.IsValidIndex(0)) 
	{
		if (DungeonGridInfo[0].IsValidIndex(0)) 
		{
			FVector Location;

			//Each cube/cell is 400x400, by getting half the array size of grid = center
			Location.X = (DungeonGridInfo.Num() / 2) * 400.f;
			Location.Y = (DungeonGridInfo[0].Num() / 2) * 400.f;
			Location.Z = 88.f;

			SetActorLocation(FVector(Location));

			SetRandomSpawnRotation();

			//Then update the navigation grid to notify that the center location is occupied by the player
			CurrentCell.X = DungeonGridInfo.Num() / 2;
			CurrentCell.Y = DungeonGridInfo[0].Num() / 2;

			//Make the spawned cell not movable for any other entity as well as update the global coords of the player
			myDungeonState->SetPlayerSpawnInformation(CurrentCell);
		}
		
	}
}

void AMC::OnMovementTimelineTick(float Alpha)
{
	FVector LerpLocation = FMath::Lerp(MoveForwardStartLocation, MoveForwardEndLocation, Alpha);

	SetActorLocation(LerpLocation);
}

void AMC::OnMovementTimelineFinished()
{
	bAbleToMove = true;
	bAlreadyMovingForward = false;
	bAlreadyMovingLeftRight = false;
	bPathBlockedButPossibleDiagonal = false;

	//Both keys still held, so do diagonal movement
	if (bMoveForwardInputStillHeldDown && bMoveLeftRightInputStillHeldDown) 
	{
		bPathBlockedButPossibleDiagonal = true;

		Manual_MoveForward();
	}
	else if (bMoveForwardInputStillHeldDown) //Continue moving forward/back
	{
		Manual_MoveForward();
	}
	else if (bMoveLeftRightInputStillHeldDown) //Continue moving left/right
	{
		Manual_MoveLeftRight();
	}
}

void AMC::OnRotate90TimelineTick(float Alpha)
{
	FRotator LerpRotation = FMath::Lerp(TimelineStartRotation, TimelineEndRotation, Alpha);

	SetActorRotation(LerpRotation);
}

void AMC::OnRotate90TimelineFinished()
{
	bAlreadyRotating = false;

	if (bRotate90StillHeldDown) 
	{
		Manual_Rotate90();
	}
}

void AMC::OnRotate180TimelineTick(float Alpha)
{
	FRotator LerpRotation = FMath::Lerp(TimelineStartRotation, TimelineEndRotation, Alpha);

	SetActorRotation(LerpRotation);
}

void AMC::OnRotate180TimelineFinished()
{
	bAbleToMove = true;

	bAlreadyRotating = false;
}

void AMC::OnRightHandMeshMovementTimelineTick(float Alpha)
{
	FTransform LerpTransform;

	LerpTransform.Blend(Temp_StartRightHandTransform, Temp_EndRightHandTransform, Alpha);

	//Send the attack message around the mid swing of the main swipe animation
	if (CurrentRightHandWeaponAnimationState == EWeaponAnimationState::BtoC 
		&& Alpha >= 0.6f && !bRightHandHit) 
	{
		//UE_LOG(LogTemp, Display, TEXT("Hitting Enemy!"));
		bRightHandHit = true;
		//Check if an enemy is one cell forward and damage it
		Attack_One_Cell_Forward();
	}

	RightArm->SetRelativeTransform(LerpTransform);
}

void AMC::OnRightHandMeshMovementTimelineFinished()
{
	//So if you are finished with a to b go to b to c
	if (CurrentRightHandWeaponAnimationState == EWeaponAnimationState::AtoB)
	{
		CurrentRightHandWeaponAnimationState = EWeaponAnimationState::BtoC;
		//Now play the new animation from the new anim state
		PlayRightHandAnimation(CurrentRightHandWeaponAnimationState, CurrentWeaponAnimIndex);
	}
	//if finished with b to c, go from c to a, during this animation it can be overwritten by left click
	//to play a to b
	else if (CurrentRightHandWeaponAnimationState == EWeaponAnimationState::BtoC) 
	{
		CurrentRightHandWeaponAnimationState = EWeaponAnimationState::CtoA;
		bRightHandIsAttacking = false;
		//Now play the new animation from the new anim state
		PlayRightHandAnimation(CurrentRightHandWeaponAnimationState, CurrentWeaponAnimIndex);
	}
	//if finished CtoA I don't want to play anything or do anything as the left click will override everything
	else if (CurrentRightHandWeaponAnimationState == EWeaponAnimationState::CtoA)
	{
		//This bool will be used to select a random start index on the next attack if player does not try
		//to combo
		bFullyCompletedRightHand = true;
	}
}

void AMC::Manual_MoveForward()
{
	FCompassDirection TempDirection = CurrentCompassDirection;

	//If the input to move was s (or something similar) you move back, so get the 180 rotation value for the direction
	if (MovementForwardBackwardChecker < 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Going Backwards"));
		TempDirection.Rotate_By_X_Amount(-180.f);
	}

	//If you cannot move forward from input, immediately return as well as set a delay for a possible diagonal that is
	//technically blocked if move forward was pressed before left/right is
	if (!myDungeonState->Can_Move_Forward(CurrentCell.X, CurrentCell.Y, TempDirection)) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Forward path blocked, Waiting for possible diagonal"));

		bPathBlockedButPossibleDiagonal = true;

		GetWorldTimerManager().SetTimer(TH_DelayedDiagonalFromPossibleBlockedPath, this,
			&AMC::Timer_DelayedBlockedDiagonalChecker, DiagonalMovementAcceptanceDelay);

		return;
	}

	UE_LOG(LogTemp, Display, TEXT("MOVING FORWARD/Backward!!!"));

	//Now move the player coords forward by checking the normalised yaw from the player, Passed by reference,
	//meaning the x and y values will be changed
	myDungeonState->Moving_Forward(this, CurrentCell.X, CurrentCell.Y, TempDirection);

	//Set to false so the timeline can do the movement animation, set to true on finished in BP
	bAbleToMove = false;

	bAlreadyMovingForward = true;

	FVector StartLocation = GetActorLocation();

	FVector EndLocation;
	EndLocation.X = CurrentCell.X * 400.f;
	EndLocation.Y = CurrentCell.Y * 400.f;
	EndLocation.Z = StartLocation.Z;

	//Set a camera shake towards the player when they move to create juice into the walking
	UGameplayStatics::PlayWorldCameraShake(
		GetWorld(),
		MoveForwardCameraShakeClass,
		GetActorLocation(),
		800.f,
		1000.f,
		1.f,
		false
	);

	//Set the start/end lerp locations which will be used in the movementtimeline event
	MoveForwardStartLocation = StartLocation;
	MoveForwardEndLocation = EndLocation;

	//Check if the Movement timeline component is valid. Sometimes Unreal has a bug where the component
	//Does not init. If failed to init, I have to reload the BP child class to force update the component
	if (MovementTimeline)
	{
		MovementTimeline->PlayFromStart();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Timeline Not Valid"));
	}

	//So move Left/Right has been blocked, but is waiting for a possible diagonal, You have moved forward right now
	// There is a very tiny, extremely, small chance that in the ~0.1 second delay something has moved to that cell
	// So check again to see if can move Left/Right whilst moving along the current timeline. If a genuine wall, nothing
	// happens
	if (bPathBlockedButPossibleDiagonal)
	{
		//Don't forget to make false or will keep moving diagonally until there's a blocked space
		bPathBlockedButPossibleDiagonal = false;

		//Since already made the bool above false, no need to have the timer below which does the same thing
		//AND Setting false above is necessary!!!
		GetWorldTimerManager().ClearTimer(TH_DelayedDiagonalFromPossibleBlockedPath);

		Manual_MoveLeftRight();
	}
}

void AMC::Manual_MoveLeftRight()
{
	FCompassDirection TempDirection = CurrentCompassDirection;

	//Clicked input to move left
	if (MovementLeftRightChecker < 0.f)
	{
		TempDirection.Rotate_By_X_Amount(-90.f);
	}
	else //else means a positive which is right (not checking for deadzone here)
	{
		TempDirection.Rotate_By_X_Amount(90.f);
	}

	//Use the temp direction as I am not updating the current compass direction as I am moving from one cell in this function
	if (myDungeonState->Can_Move_Forward(CurrentCell.X, CurrentCell.Y, TempDirection))
	{
		//x and y get passed in as reference so they get updated when moving from one cell to the other
		myDungeonState->Moving_Forward(this, CurrentCell.X, CurrentCell.Y, TempDirection);

		//stop all other movement events until the timeline is finished
		bAbleToMove = false;

		bAlreadyMovingLeftRight = true;

		FVector StartLocation = GetActorLocation();

		FVector EndLocation;
		EndLocation.X = CurrentCell.X * 400.f;
		EndLocation.Y = CurrentCell.Y * 400.f;
		EndLocation.Z = StartLocation.Z;

		//FVector EndLocation = GetActorLocation() + (GetActorRightVector() * EndLocationRightVectorAdder);
		//EndLocation.Z = StartLocation.Z;

		//Set a camera shake towards the player when they move to create juice into the walking
		UGameplayStatics::PlayWorldCameraShake(
			GetWorld(),
			MoveForwardCameraShakeClass,
			GetActorLocation(),
			800.f,
			1000.f,
			1.f,
			false
		);

		//Set the start/end lerp locations which will be used in the movementtimeline event
		MoveForwardStartLocation = StartLocation;
		MoveForwardEndLocation = EndLocation;

		//Check if the Movement timeline component is valid. Sometimes Unreal has a bug where the component
		//Does not init. If failed to init, I have to reload the BP child class to force update the component
		if (MovementTimeline)
		{
			MovementTimeline->PlayFromStart();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Timeline Not Valid"));
		}

		//So move forward has been blocked, but is waiting for a possible diagonal, You have moved left right now
		// There is a very tiny, extremely, small chance that in the ~0.1 second delay something has moved to that cell
		// So check again to see if can move forward whilst moving along the current timeline. If a genuine wall, nothing
		// happens
		if (bPathBlockedButPossibleDiagonal) 
		{
			//Don't forget to make false or will keep moving diagonally until there's a blocked space
			bPathBlockedButPossibleDiagonal = false;

			//Since already made the bool above false, no need to have the timer below which does the same thing
			//AND Setting false above is necessary!!!
			GetWorldTimerManager().ClearTimer(TH_DelayedDiagonalFromPossibleBlockedPath);

			Manual_MoveForward();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Left Right path blocked, Waiting for possible diagonal"));

		bPathBlockedButPossibleDiagonal = true;

		GetWorldTimerManager().SetTimer(TH_DelayedDiagonalFromPossibleBlockedPath, this,
			&AMC::Timer_DelayedBlockedDiagonalChecker, DiagonalMovementAcceptanceDelay);
	}
}

void AMC::Manual_Rotate90()
{
	if (bAlreadyRotating) return;

	bAlreadyRotating = true;
	FRotator Desired_Rotation = GetActorRotation();

	//Rotate -90 (left)
	if (RotationLeftRightChecker < 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Rotating Left!"));
		//struct function that handles the casting of my compass enum to rotate my current rotation
		CurrentCompassDirection.Rotate_By_X_Amount(-90.f);

		//Rotate along the yaw to the left
		Desired_Rotation.Yaw -= 90.f;

	}
	else if (RotationLeftRightChecker > 0.f)//Rotating 90 degrees right
	{
		UE_LOG(LogTemp, Warning, TEXT("Rotating Right!"));
		//struct function that handles the casting of my compass enum to rotate my current rotation
		CurrentCompassDirection.Rotate_By_X_Amount(90.f);

		//Rotate along the yaw to rotate right
		Desired_Rotation.Yaw += 90.f;
	}

	//Set the start and end lerp rotations which will be used in the rotate90 timeline event
	TimelineStartRotation = GetActorRotation();
	TimelineEndRotation = Desired_Rotation;

	//Check if the timeline component is valid, Had an Unreal bug where the component was not initting which would cause
	//Crashes, if happens again I can localise issue and know that I have to reload my bp child class
	if (Rotate90Timeline)
	{
		Rotate90Timeline->PlayFromStart();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Rotate90 Timeline Not Valid"));
	}
}

void AMC::Timer_DelayedBlockedDiagonalChecker()
{
	//Have a timer event that checks if a diagonal movement is possible if a path is blocked. i.e w+d may work but
	// d + w might not work because the d has a blocked path. So, delay the two inputs for this majiggy to work.
	bPathBlockedButPossibleDiagonal = false;

	UE_LOG(LogTemp, Warning, TEXT("Possible Diagonal timer has finished!"));
}

void AMC::MoveForwardAndBackward(const FInputActionValue& Value)
{
	//Used for timelines, once forward lerp is finished, if the move forward key is still pressed, it will call movement
	//again, This does assume that the IA class has the OnPressed + OnReleased trigger
	bMoveForwardInputStillHeldDown = Value.Get<bool>();

	MovementForwardBackwardChecker = Value.Get<float>();

	//Use 2 bools for holding the key down because I used timeline for movement and one to fire move forward only once when
	//key down (Cannot use onpressed because w down -> s down -> w released would continue moving forward)
	if (!bMoveForwardInputStillHeldDown) 
	{
		bMoveForwardBackwardWaitingForRelease = false;

		return;
	} 

	//the whole movement logic relies on the game state, so instantly shoot red flags if not valid
	if (!myDungeonState) 
	{
		UE_LOG(LogTemp, Error, TEXT("Dungeon Game State not found!!!"));
		return;
	} 

	//if already moving from another function or this, instantly return and don't do logic below
	if (bAlreadyMovingForward)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Already Moving Forward!"));
		return;
	}

	//UE_LOG(LogTemp, Warning, TEXT("Move forward/back key down, current x = %f"), MovementForwardBackwardChecker);
	
	//Since I switched to on down instead of on pressed, I need a bool to call manual move forward once
	if (bMoveForwardBackwardWaitingForRelease) 
	{
		return;
	}
	else 
	{
		bMoveForwardBackwardWaitingForRelease = true;
		//Call the C++ logic to move forward
		Manual_MoveForward();
	}	
}

void AMC::MoveLeftRight(const FInputActionValue& Value)
{
	//Used for timelines, once forward lerp is finished, if the move forward key is still pressed, it will call movement
	//again, This does assume that the IA class has the OnPressed + OnReleased trigger
	bMoveLeftRightInputStillHeldDown = Value.Get<bool>();

	MovementLeftRightChecker = Value.Get<float>();

	//Use 2 bools for holding the key down because I used timeline for movement and one to fire move forward only once when
	//key down (Cannot use onpressed because w down -> s down -> w released would continue moving forward)
	if (!bMoveLeftRightInputStillHeldDown)
	{
		bMoveLeftRightWaitingForRelease = false;

		return;
	}

	//the whole movement logic relies on the game state, so instantly shoot red flags if not valid
	if (!myDungeonState)
	{
		//UE_LOG(LogTemp, Error, TEXT("Dungeon Game State not found!!!"));
		return;
	}

	//if already moving from another function or this, instantly return and don't do logic below
	if (bAlreadyMovingLeftRight)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Already Moving Left/Right!"));
		return;
	}

	if (bMoveLeftRightWaitingForRelease) 
	{
		return;
	}
	else 
	{
		bMoveLeftRightWaitingForRelease = true;

		Manual_MoveLeftRight();
	}
}

void AMC::RotateLeftRight(const FInputActionValue& Value)
{
	RotationLeftRightChecker = Value.Get<float>();

	//Used For the onfinished rotate90 timeline to keep rotating if the key is still held down
	bRotate90StillHeldDown = Value.Get<bool>();

	if (!bRotate90StillHeldDown) 
	{
		//This input will be on tick because of "ondown" but I only want to fire once when the key is pressed
		bRotate90WaitingForRelease = false;
		return;
	}

	if (bAlreadyRotating) return;

	if (bMoveLeftRightWaitingForRelease)
	{
		return;
	}
	else 
	{
		UE_LOG(LogTemp, Display, TEXT("Input read Rotate Left Right"));

		bRotate90WaitingForRelease = true;
		Manual_Rotate90();
	}
}

void AMC::Rotate180(const FInputActionValue& Value)
{
	//if already moving from another function or this, instantly return and don't do logic below
	if (bAlreadyRotating)
	{
		UE_LOG(LogTemp, Warning, TEXT("CURRENTLY MOVING OR FIGHTING"));
		return;
	}

	UE_LOG(LogTemp, Display, TEXT("Movement Back"));
	FRotator Desired_Rotation = GetActorRotation();

	UE_LOG(LogTemp, Display, TEXT("Right ROTATING"));
	//Rotate along the Yaw to rotate the player behind
	Desired_Rotation.Yaw += 180.f;

	//struct function that handles the casting of my compass enum to rotate my current rotation
	CurrentCompassDirection.Rotate_By_X_Amount(180.f);

	//Old Code when I had bp timeline logic
	//Call_Rotate_180(GetActorRotation(), Desired_Rotation);

	//Since an animation is going to start, I have to make able to move false so that no other inputs can intefere
	bAbleToMove = false;

	bAlreadyRotating = true;

	//Set the start and end lerp rotations which will be used in the 180 timeline event
	TimelineStartRotation = GetActorRotation();
	TimelineEndRotation = Desired_Rotation;

	//Check if the timeline component is valid, Had an Unreal bug where the component was not initting which would cause
	//Crashes, if happens again I can localise issue and know that I have to reload my bp child class
	if (Rotate180Timeline)
	{
		Rotate180Timeline->PlayFromStart();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Rotate180 Timeline Not Valid"));
	}
}

void AMC::Interacted(const FInputActionValue& Value)
{
	//Get the player controller
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();

	FMinimalViewInfo CamInfo;

	myCamera->GetCameraView(0.f, CamInfo);

	FVector WorldLocation = CamInfo.Location;

	FVector EndLocation = CamInfo.Rotation.Vector();

	EndLocation.Normalize();

	FVector LineTraceEnd = WorldLocation + (EndLocation * 400.f);

	//Passed as a reference so has to be created here
	FHitResult HitResult;
	//same here
	FCollisionQueryParams CollisionParams;

	//ignore the fact that the line trace might hit the self actor
	CollisionParams.AddIgnoredActor(this);

	//The actual line trace being created
	if (GetWorld()->LineTraceSingleByChannel(HitResult, WorldLocation, LineTraceEnd,
		ECollisionChannel::ECC_Visibility, CollisionParams))
	{
		//DrawDebugLine(GetWorld(), WorldLocation,
			//LineTraceEnd, FColor::Red, false, 2.f, 0, 0.1f);

		if (HitResult.GetActor()) 
		{
			if (HitResult.GetActor()->ActorHasTag("POI")) 
			{
				IPOI_Interaction* POIComms = Cast<IPOI_Interaction>(HitResult.GetActor());

				if (POIComms)
				{
					POIComms->Interacted();
				}
			}
			
		}
	}
}

void AMC::RightAttack(const FInputActionValue& Value)
{
	if (!bRightHandIsAttacking) 
	{
		//Set a camera shake when attacking
		UGameplayStatics::PlayWorldCameraShake(
			GetWorld(),
			AttackShake,
			GetActorLocation(),
			800.f,
			1000.f,
			1.f,
			false
		);

		UE_LOG(LogTemp, Display, TEXT("Right Attack Pressed!"));
		//Have a bool that stops the left click from interuppting the lerp anim whilst it's mid way
		//(Will be able to left click on the CtoA weapon state though!!!) -> For cool gameplay reasons...
		bRightHandIsAttacking = true;
		bRightHandHit = false;

		//If fully completed a combo, randomly have the start index be somewhere within the available animations
		if (bFullyCompletedRightHand) 
		{
			//Turn to false, as I want the ctoa event to finish to reset this to true
			bFullyCompletedRightHand = false;
			//Choose a random attack animation to play
			int RandIndex = FMath::RandRange(0, RightHandAttackAnimations.Num() - 1);
			//- by the index because you add the CurrentWeaponIndex below
			CurrentWeaponAnimIndex = RandIndex;
			//Debug
			//RandIndex = 1;
		}

		//Have a global index for the current weapon anim because this will be used at the onfinished for the right hand
		//timeline to pick the next weapon anim state
		//CurrentWeaponAnimIndex = RandIndex;
		CurrentWeaponAnimIndex = (CurrentWeaponAnimIndex + 1) % RightHandAttackAnimations.Num();
		//Debug
		//CurrentWeaponAnimIndex = 2;

		//Use a helper function that decides the complicated logic for what each AI animation state should do for the
		//timeline (The currentweaponanimindex will be checked to see if it fits in the weapon anim array!!!)
		PlayRightHandAnimation(EWeaponAnimationState::AtoB, CurrentWeaponAnimIndex);
	}
}

// Called every frame
void AMC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMC::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* AddInputAction = Cast<UEnhancedInputComponent>(InputComponent))
	{
		AddInputAction->BindAction(IA_MoveForwardAndBackward, ETriggerEvent::Triggered, this, &AMC::MoveForwardAndBackward);
		AddInputAction->BindAction(IA_MoveLeftRight, ETriggerEvent::Triggered, this, &AMC::MoveLeftRight);
		AddInputAction->BindAction(IA_RotateLeftRight, ETriggerEvent::Triggered, this, &AMC::RotateLeftRight);
		AddInputAction->BindAction(IA_RotateLeftRight180, ETriggerEvent::Triggered, this, &AMC::Rotate180);
		AddInputAction->BindAction(IA_Interacted, ETriggerEvent::Triggered, this, &AMC::Interacted);
		AddInputAction->BindAction(IA_RightAttack, ETriggerEvent::Triggered, this, &AMC::RightAttack);
	}
}

void AMC::Attack_One_Cell_Forward()
{
	//Make an array for all the possible damage cells, attacking one cell forward just has one, the function call right at
	//the end of this function needs an array of damage cells as I am planning for aoe attacks
	TArray<FIntPoint> DamageArray = { CurrentCell };
	FIntPoint& AttackCell = DamageArray[0];

	switch (CurrentCompassDirection.Get_Current_Compass_Direction()) 
	{
	case FCompassDirection::ECompassDirection::North:
		AttackCell.Y--;
		break;

	case FCompassDirection::ECompassDirection::South:
		AttackCell.Y++;
		break;

	case FCompassDirection::ECompassDirection::West:
		AttackCell.X--;
		break;

	case FCompassDirection::ECompassDirection::East:
		AttackCell.X++;
		break;
	}

	myDungeonState->Try_Sending_Damage_To_Entity(DamageArray, 999.f);
}

void AMC::SetRandomSpawnRotation()
{
	//Unreal's rotation system is crap so I am brute forcing my normalised rotation for every actor spawned to face
	//0,0 of the grid which is the top left, testing I found yaw -90 faces that with my player so set it to this
	//Once I am comfortable with the navigation working, I will come back to this when I figure out how to 
	//translate relative rotations with 0 being forward on the navigation grid (This will be priority when coding
	//AI)
	FRotator CustomRotationSpawn (0.0f, 0.0f, 0.0f); // (Came back and settled for enums for internal rotations)
	

	int RandSpawn = FMath::RandRange(0, 3);

	switch (RandSpawn) 
	{
	case 0:
		//In world -90 points to North
		CustomRotationSpawn.Yaw = -90;
		//The setting of the enum looks weird but I encapsulated my enum within my struct mainly for the enum size in
		//rotation wrapping which I don't want anything else to intefere with
		CurrentCompassDirection.Set_Compass_Direction(FCompassDirection::ECompassDirection::North);
		break;

	case 1:
		//In world 0.0 points to East
		CustomRotationSpawn.Yaw = 0.0f;
		CurrentCompassDirection.Set_Compass_Direction(FCompassDirection::ECompassDirection::East);
		break;

	case 2:
		//In world 90.0 points to South
		CustomRotationSpawn.Yaw = 90.0f;
		CurrentCompassDirection.Set_Compass_Direction(FCompassDirection::ECompassDirection::South);
		break;

	case 3:
		//In world 180.0 points to West
		CustomRotationSpawn.Yaw = 180.0f;
		CurrentCompassDirection.Set_Compass_Direction(FCompassDirection::ECompassDirection::West);
		break;
	}

	SetActorRotation(FQuat(CustomRotationSpawn));
}

void AMC::SetRightHandAnimationPlayRate(float InTime)
{
	RightHandAnimationTimeline->SetPlayRate(1 / InTime);
}

void AMC::PlayRightHandAnimation(EWeaponAnimationState AnimationToPlay, int WeaponAnimationIndex)
{
	//Check if the inputted index for searching the weapon animation transforms etc is valid
	if (!RightHandAttackAnimations.IsValidIndex(WeaponAnimationIndex)) 
	{
		UE_LOG(LogTemp, Error, TEXT("Inputted weapon index (%d) is not valid, check the MC right hand anim array!"),
			WeaponAnimationIndex);
		return;
	}

	UE_LOG(LogTemp, Display, TEXT("Going To Play Right Hand Weapon Anim!"));

	float AttackTime;

	//Essentially, when I want to play the right hand weapon animation, I want to reuse the same timeline to reduce overhead
	//In doing so, I have different states that need to be picked to set the start and end transform for the timeline to
	//lerp through. Instead of checking each tick for what the current weapon anim state is, I do it before
	switch (AnimationToPlay) 
	{
	case EWeaponAnimationState::AtoB:
		//Set the start transform to the relative transform of rightarm mesh, do this because I want the attack system
		//to allow the left click to do the next weapon animation as the weapon is going back to the idle position
		//This will be during the CtoA state as this will be a slower timeline than the rest
		Temp_StartRightHandTransform = RightArm->GetRelativeTransform();

		Temp_EndRightHandTransform = RightHandAttackAnimations[WeaponAnimationIndex].BTransform;

		//This is important to set here as I will use this when the timeline is finished to decide what the next animation
		//will be
		CurrentRightHandWeaponAnimationState = AnimationToPlay;

		AttackTime = RightHandAttackAnimations[WeaponAnimationIndex].AtoBTime;
		if (RightHandAnimationTimeline->IsPlaying()) 
		{
			UE_LOG(LogTemp, Warning, TEXT("Sword Playing Animation Currently, Slowing Down!"));
			AttackTime *= 2.f;
		}

		//Set the play rate of the timeline through a function helper to avoid confusion for the timing of the animation
		SetRightHandAnimationPlayRate(AttackTime);

		//Now, simply play
		RightHandAnimationTimeline->PlayFromStart();

		break;
	
	//All the other weapon animation states are the same as the above essentially
	case EWeaponAnimationState::BtoC:

		Temp_StartRightHandTransform = RightHandAttackAnimations[WeaponAnimationIndex].BTransform;

		Temp_EndRightHandTransform = RightHandAttackAnimations[WeaponAnimationIndex].CTransform;

		CurrentRightHandWeaponAnimationState = AnimationToPlay;

		SetRightHandAnimationPlayRate(RightHandAttackAnimations[WeaponAnimationIndex].BtoCTime);

		RightHandAnimationTimeline->PlayFromStart();

		break;

	case EWeaponAnimationState::CtoA:

		Temp_StartRightHandTransform = RightHandStartTransform;

		//Don't use the right hand attack animation array here but the global variable instead. Do this because
		//I want to avoid repetition of recreating the same A transform on each new weapon attack animation in BP
		Temp_EndRightHandTransform = RightHandAttackAnimations[WeaponAnimationIndex].CTransform;;

		CurrentRightHandWeaponAnimationState = AnimationToPlay;

		SetRightHandAnimationPlayRate(RightHandAttackAnimations[WeaponAnimationIndex].CtoATime);

		RightHandAnimationTimeline->ReverseFromEnd();

		break;

	default:
		UE_LOG(LogTemp, Error, TEXT("Something went horribly wrong in MC::PlayRightHandAnimation function!!!"));
	}
}

