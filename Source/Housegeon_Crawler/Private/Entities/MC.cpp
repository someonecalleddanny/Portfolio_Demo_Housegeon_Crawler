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

		RightHandAnimationTimeline->AddInterpFloat(MovementFloatCurve, RightHandMeshMovementInterp);
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
			myGridTransform.X = DungeonGridInfo.Num() / 2;
			myGridTransform.Y = DungeonGridInfo[0].Num() / 2;

			OldCell.X = DungeonGridInfo.Num() / 2;
			OldCell.Y = DungeonGridInfo[0].Num() / 2;

			//Make the spawned cell not movable for any other entity as well as update the global coords of the player
			myDungeonState->SetPlayerSpawnInformation(OldCell);
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

	if (bKeepWalkingForward) 
	{
		Manual_MoveForward();
	}
}

void AMC::OnRotate90TimelineTick(float Alpha)
{
	FRotator LerpRotation = FMath::Lerp(TimelineStartRotation, TimelineEndRotation, Alpha);

	SetActorRotation(LerpRotation);
}

void AMC::OnRotate90TimelineFinished()
{
	bAbleToMove = true;
}

void AMC::OnRotate180TimelineTick(float Alpha)
{
	FRotator LerpRotation = FMath::Lerp(TimelineStartRotation, TimelineEndRotation, Alpha);

	SetActorRotation(LerpRotation);
}

void AMC::OnRotate180TimelineFinished()
{
	bAbleToMove = true;
}

void AMC::OnRightHandMeshMovementTimelineTick(float Alpha)
{
	FTransform LerpTransform;

	LerpTransform.Blend(Temp_StartRightHandTransform, Temp_EndRightHandTransform, Alpha);

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
	
}

void AMC::Manual_MoveForward()
{
	//Check if the forward cell (check rotation from normalised yaw, set when calling rotation functions),
	//is movable
	if (myDungeonState->Can_Move_Forward(myGridTransform.X, myGridTransform.Y, myGridTransform.NormalizedYaw))
	{
		UE_LOG(LogTemp, Display, TEXT("MOVING FORWARD!!!"));

		//Make the previous cell movable for all entities
		myDungeonState->UpdateOldMovementCell(OldCell);

		//Now move the player coords forward by checking the normalised yaw from the player, Passed by reference,
		//meaning the x and y values will be changed
		myDungeonState->Moving_Forward(myGridTransform.X, myGridTransform.Y, myGridTransform.NormalizedYaw);

		//After the coords above have been changed, update the global player coords for the enemy AI to track where the
		//player is
		myDungeonState->UpdatePlayerCoords(this, FIntPoint(myGridTransform.X, myGridTransform.Y));

		//Update the new old cell to the current cell that the player is within
		OldCell.X = myGridTransform.X;
		OldCell.Y = myGridTransform.Y;

		//Set to false so the timeline can do the movement animation, set to true on finished in BP
		bAbleToMove = false;

		FVector StartLocation = GetActorLocation();

		//Get the end location by getting the forward vector + start location. Don't change the z as only moving on
		//x, y axes
		FVector EndLocation = GetActorLocation() + (GetActorForwardVector() * 400.f);
		EndLocation.Z = StartLocation.Z;

		//Call_Move_Forward(StartLocation, EndLocation);

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
		
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Forward path blocked, can't move forward"))
	}
}

void AMC::MoveForward(const FInputActionValue& Value)
{
	//Used for timelines, once forward lerp is finished, if the move forward key is still pressed, it will call movement
	//again, This does assume that the IA class has the OnPressed + OnReleased trigger
	bKeepWalkingForward = Value.Get<bool>();

	//the whole movement logic relies on the game state, so instantly shoot red flags if not valid
	if (!myDungeonState) 
	{
		UE_LOG(LogTemp, Error, TEXT("Dungeon Game State not found!!!"));
		return;
	} 

	//if already moving from another function or this, instantly return and don't do logic below
	if (!bAbleToMove)
	{
		UE_LOG(LogTemp, Warning, TEXT("CURRENTLY MOVING OR FIGHTING"));
		return;
	}

	//Call the C++ logic to move forward
	Manual_MoveForward();
}

void AMC::RotateLeftRight(const FInputActionValue& Value)
{

	//if already moving from another function or this, instantly return and don't do logic below
	if (!bAbleToMove)
	{
		UE_LOG(LogTemp, Warning, TEXT("CURRENTLY MOVING OR FIGHTING"));
		return;
	}

	UE_LOG(LogTemp, Display, TEXT("Movement Left Right"));
	float LeftRightChecker = Value.Get<float>();
	FRotator Desired_Rotation = GetActorRotation();

	//Rotate -90 (left)
	if (LeftRightChecker < 0.f)
	{
		UE_LOG(LogTemp, Display, TEXT("LEFT ROTATING, current rotation = %f"), GetActorRotation().Yaw);

		//The temp normalised yaw system to make sure that the grid navigation is working (will fix on later stage of
		//development, AI system)
		myGridTransform.NormalizedYaw -= 90.f;

		//if started on the forward rotation but moved left, go to WEST which is 270.f,
		// do this to force 4 yaw values 0, 90 , 180, 270
		if (myGridTransform.NormalizedYaw < 0) 
		{
			myGridTransform.NormalizedYaw = 270.f;
		}

		//Rotate along the yaw to the left
		Desired_Rotation.Yaw -= 90.f;
		
		//Old BP logic
		//Call_Rotate_90(GetActorRotation(), Desired_Rotation);

		//Rotate the normalised yaw left for the grid naviation system (Currently a temp solution will come back later)
	}
	else//Rotating 90 degrees right
	{
		//The temp normalised yaw system to make sure that the grid navigation is working (will fix on later stage of
		//development, AI system)
		myGridTransform.NormalizedYaw += 90.f;
		
		//if on WEST (270.f) degrees on yaw, Set it to NORTH which is 0.f, do this to force 4 yaw values 0, 90 , 180, 270
		if (myGridTransform.NormalizedYaw > 270.f)
		{
			myGridTransform.NormalizedYaw = 0.f;
		}

		//Rotate along the yaw to rotate right
		Desired_Rotation.Yaw += 90.f;

		UE_LOG(LogTemp, Display, TEXT("Right ROTATING, current rotation = %f"), GetActorRotation().Yaw);
	}

	//Once finished with determining the new rotation from reading if going left right from A and D key inputs, do the
	//C++ Timeline to rotate the player (I first did a BP timeline but now converted it into a C++ one)
	bAbleToMove = false;

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

void AMC::Rotate180(const FInputActionValue& Value)
{
	//if already moving from another function or this, instantly return and don't do logic below
	if (!bAbleToMove)
	{
		UE_LOG(LogTemp, Warning, TEXT("CURRENTLY MOVING OR FIGHTING"));
		return;
	}

	UE_LOG(LogTemp, Display, TEXT("Movement Back"));
	FRotator Desired_Rotation = GetActorRotation();

	UE_LOG(LogTemp, Display, TEXT("Right ROTATING"));
	//Rotate along the Yaw to rotate the player behind
	Desired_Rotation.Yaw += 180.f;

	//Do the Navigation grid transform
	myGridTransform.NormalizedYaw += 180.f;

	//If goes over 270, normalise by subtrating by 360
	if (myGridTransform.NormalizedYaw > 270.f)
	{
		myGridTransform.NormalizedYaw -= 360.f;
	}

	//Old Code when I had bp timeline logic
	//Call_Rotate_180(GetActorRotation(), Desired_Rotation);

	//Since an animation is going to start, I have to make able to move false so that no other inputs can intefere
	bAbleToMove = false;

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

	FVector LineTraceEnd = WorldLocation + (EndLocation * 500.f);

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
		DrawDebugLine(GetWorld(), WorldLocation,
			LineTraceEnd, FColor::Red, false, 2.f, 0, 0.1f);

		if (HitResult.GetActor()) 
		{
			if (HitResult.GetActor()->ActorHasTag("POI")) 
			{
				IPOI_Interaction* POIInterface = Cast<IPOI_Interaction>(HitResult.GetActor());

				if (POIInterface) 
				{
					POIInterface->Interacted();
				}
			}
		}
	}
}

void AMC::RightAttack(const FInputActionValue& Value)
{
	if (!bRightHandIsAttacking) 
	{
		UE_LOG(LogTemp, Display, TEXT("Right Attack Pressed!"));
		//Have a bool that stops the left click from interuppting the lerp anim whilst it's mid way
		//(Will be able to left click on the CtoA weapon state though!!!) -> For cool gameplay reasons...
		bRightHandIsAttacking = true;

		//Choose a random attack animation to play
		int RandIndex = FMath::RandRange(0, RightHandAttackAnimations.Num() - 1);
		//Debug
		RandIndex = 0;

		//Have a global index for the current weapon anim because this will be used at the onfinished for the right hand
		//timeline to pick the next weapon anim state
		CurrentWeaponAnimIndex = RandIndex;

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
		AddInputAction->BindAction(IA_MoveForward, ETriggerEvent::Triggered, this, &AMC::MoveForward);
		AddInputAction->BindAction(IA_RotateLeftRight, ETriggerEvent::Triggered, this, &AMC::RotateLeftRight);
		AddInputAction->BindAction(IA_RotateLeftRight180, ETriggerEvent::Triggered, this, &AMC::Rotate180);
		AddInputAction->BindAction(IA_Interacted, ETriggerEvent::Triggered, this, &AMC::Interacted);
		AddInputAction->BindAction(IA_RightAttack, ETriggerEvent::Triggered, this, &AMC::RightAttack);
	}
}

void AMC::SetRandomSpawnRotation()
{
	//Unreal's rotation system is crap so I am brute forcing my normalised rotation for every actor spawned to face
	//0,0 of the grid which is the top left, testing I found yaw -90 faces that with my player so set it to this
	//Once I am comfortable with the navigation working, I will come back to this when I figure out how to 
	//translate relative rotations with 0 being forward on the navigation grid (This will be priority when coding
	//AI)
	FRotator CustomRotationSpawn (0.0f, 0.0f, 0.0f);
	

	int RandSpawn = FMath::RandRange(0, 3);

	switch (RandSpawn) 
	{
	case 0:
		CustomRotationSpawn.Yaw = -90;
		//Create a normalised rotation, (useful for when checking navigation grid), 0 means forward
		myGridTransform.NormalizedYaw = 0.f;
		break;

	case 1:
		CustomRotationSpawn.Yaw = 0.0f;
		//Create a normalised rotation, (useful for when checking navigation grid), 90 means right
		myGridTransform.NormalizedYaw = 90.f;
		break;

	case 2:
		CustomRotationSpawn.Yaw = 90.0f;
		//Create a normalised rotation, (useful for when checking navigation grid), 0 means back
		myGridTransform.NormalizedYaw = 180.f;
		break;

	case 3:
		CustomRotationSpawn.Yaw = 180.0f;
		//Create a normalised rotation, (useful for when checking navigation grid), 0 means back
		myGridTransform.NormalizedYaw = 270.f;
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

		//Set the play rate of the timeline through a function helper to avoid confusion for the timing of the animation
		SetRightHandAnimationPlayRate(RightHandAttackAnimations[WeaponAnimationIndex].AtoBTime);

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

		Temp_StartRightHandTransform = RightHandAttackAnimations[WeaponAnimationIndex].CTransform;

		//Don't use the right hand attack animation array here but the global variable instead. Do this because
		//I want to avoid repetition of recreating the same A transform on each new weapon attack animation in BP
		Temp_EndRightHandTransform = RightHandStartTransform;

		CurrentRightHandWeaponAnimationState = AnimationToPlay;

		SetRightHandAnimationPlayRate(RightHandAttackAnimations[WeaponAnimationIndex].CtoATime);

		RightHandAnimationTimeline->PlayFromStart();

		break;

	default:
		UE_LOG(LogTemp, Error, TEXT("Something went horribly wrong in MC::PlayRightHandAnimation function!!!"));
	}
}

