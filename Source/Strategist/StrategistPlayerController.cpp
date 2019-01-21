// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "Strategist.h"
#include "StrategistPlayerController.h"
#include "AI/Navigation/NavigationSystem.h"
#include "Runtime/Engine/Classes/Components/DecalComponent.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"
#include "StrategistCharacter.h"

#include "CenterPawn.h"
#include "CheckPoint.h"
#include "StrategistGameMode.h"
#include "StrategistGameInstance.h"

DEFINE_LOG_CATEGORY(PlayerController)

AStrategistPlayerController::AStrategistPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;

	//Character BP
	ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDownCPP/Blueprints/TopDownCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
		BP_Player = PlayerPawnBPClass.Class;

	ConstructorHelpers::FClassFinder<APawn> CenterPawnBPClass(TEXT("/Game/TopDownCPP/Blueprints/BP_Center"));
	if (CenterPawnBPClass.Class != NULL)
		BP_Center = CenterPawnBPClass.Class;

	//get blueprints
	static ConstructorHelpers::FObjectFinder<UBlueprint>
		CheckPointBP(TEXT("Blueprint'/Game/TopDownCPP/Blueprints/CheckPoint_BP.CheckPoint_BP'"));
	if (CheckPointBP.Object)
	{
		checkpointBP = (UClass*)CheckPointBP.Object->GeneratedClass;
	}

	static ConstructorHelpers::FObjectFinder<UBlueprint>
		CheckPointBP_startPoint(TEXT("Blueprint'/Game/TopDownCPP/Blueprints/CheckPoint_BP2.CheckPoint_BP2'"));
	if (CheckPointBP_startPoint.Object)
	{
		checkpointBP_startPoint = (UClass*)CheckPointBP_startPoint.Object->GeneratedClass;
	}

	static ConstructorHelpers::FObjectFinder<UBlueprint>
		CheckPointBP_endPoint(TEXT("Blueprint'/Game/TopDownCPP/Blueprints/CheckPoint_Couple.CheckPoint_Couple'"));
	if (CheckPointBP_endPoint.Object)
	{
		checkpointBP_endPoint = (UClass*)CheckPointBP_endPoint.Object->GeneratedClass;
	}

	static ConstructorHelpers::FObjectFinder<UBlueprint>
		LineSphereBP(TEXT("Blueprint'/Game/Blueprints/LineSphere_BP.LineSphere_BP'"));
	if (LineSphereBP.Object)
	{
		UE_LOG(PlayerController, Log, TEXT("Found BP : LineSphere_BP"));
		lineSphereBP = (UClass*)LineSphereBP.Object->GeneratedClass;
	}

	pickedCheckPoint = nullptr;
	gameType = EGameState::EEditMode; // edit¸ðµå
	//initCharacterPos = GetPawn()->GetActorLocation();
}
void AStrategistPlayerController::BeginPlay() {
	Super::BeginPlay();

	AddCheckPoint(FVector(0, -1500, 51), checkpointBP_startPoint, 0);
	AddCheckPoint(FVector(0, 1500, 51), checkpointBP_endPoint, 1);
	MakeLine(1, EDIT_STATES::EADD);
}
void AStrategistPlayerController::Morph()
{
	//References to our different morphs

	//We destroy our pawn and unpossess it
	FVector position = GetPawn()->GetActorLocation();
	FRotator orientation = GetPawn()->GetActorRotation();

	if (GetPawn()->GetClass() == BP_Player)
	{
		GetPawn()->Destroy();
		UnPossess();

		UWorld* const World = GetWorld();

		FVector* tempP = new FVector(0, 0, characterZ);
		FRotator* tempR = new FRotator(0, 0, 0);
		//Now we spawn the other one and possess it.
		ACenterPawn* Roller = World->SpawnActor<ACenterPawn>(BP_Center, *tempP, *tempR);
		Possess(Roller);
		gameType = EGameState::EEditMode;
	}

	else if (GetPawn()->GetClass() == BP_Center)
	{
		GetPawn()->Destroy();
		UnPossess();

		UWorld* const World = GetWorld();

		//Now we spawn the other one and possess it.
		FVector* tempP = new FVector(0, -1500, characterZ);
		FRotator* tempR = new FRotator(0, 640, 0);
		AStrategistCharacter* Roller = World->SpawnActor<AStrategistCharacter>(BP_Player, *tempP, *tempR);
		Possess(Roller);
		gameType = EGameState::ESimulating;
	}
}
void AStrategistPlayerController::resetSimulationParams() {
	isSimulated = false;
	listIter = 0;
}
void AStrategistPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	//if user pick checkpoint and drag
	if (pickedCheckPoint != nullptr && dragMode)
	{
		FHitResult Hit;
		GetHitResultUnderCursor(ECC_Visibility, false, Hit);

		pickedCheckPoint->SetActorLocation(Hit.Location);
		MakeLine(pickedIndex, EDIT_STATES::EDRAG);
	}
}

void AStrategistPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	InputComponent->BindAction("SetDestination", IE_Pressed, this, &AStrategistPlayerController::OnSetDestinationPressed);
	InputComponent->BindAction("SetDestination", IE_Released, this, &AStrategistPlayerController::OnSetDestinationReleased);
	InputComponent->BindAction("StartSimulation", IE_Released, this, &AStrategistPlayerController::OnSimulationStarted);
	InputComponent->BindAction("ResetSimulation", IE_Released, this, &AStrategistPlayerController::OnSimulationRestarted);
	InputComponent->BindAction("DeleteCheckPoint", IE_Released, this, &AStrategistPlayerController::OnDeleteCheckPoint);
	InputComponent->BindAction("SetNewLevel", IE_Released, this, &AStrategistPlayerController::SetNewLevel);

	// support touch devices 
	InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AStrategistPlayerController::MoveToTouchLocation);
	InputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AStrategistPlayerController::MoveToTouchLocation);

	InputComponent->BindAction("ResetVR", IE_Pressed, this, &AStrategistPlayerController::OnResetVR);
}

void AStrategistPlayerController::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AStrategistPlayerController::MoveToMouseCursor()
{
	// Trace to see what is under the mouse cursor
	FHitResult Hit;
	GetHitResultUnderCursor(ECC_Visibility, false, Hit);

	if (Hit.bBlockingHit)
	{
		// We hit something, move there
		SetNewMoveDestination(Hit.ImpactPoint);
	}
}

void AStrategistPlayerController::myTimer() {
	isSimulated = true;
	APawn* const MyPawn = GetPawn();

	if (listIter == checkpoints.Num()) {
		setGameType(EGameState::ESimulationSuccessed);

		//for turn on success sound
		APawn* const MyPawn = GetPawn();
		((AStrategistCharacter*)MyPawn)->AfterSuccess_BP();

		resetSimulationParams();
		//Morph();
		return;
	}
	UNavigationSystem* const NavSys = GetWorld()->GetNavigationSystem();
	float const Distance = FVector::Dist(checkpoints[listIter]->GetActorLocation(), MyPawn->GetActorLocation());
	UE_LOG(PlayerController, Log, TEXT("characterLocation : %f %f %f"), MyPawn->GetActorLocation().X, MyPawn->GetActorLocation().Y, MyPawn->GetActorLocation().Z);

	float characterSpeed = ((ACharacter*)MyPawn)->GetCharacterMovement()->MaxWalkSpeed;

	NavSys->SimpleMoveToLocation(this, (checkpoints[listIter]->GetActorLocation()));
	NavSys->RemoveFromRoot();

	listIter++;
	float time = Distance / characterSpeed;
	UE_LOG(PlayerController, Log, TEXT("distance : %f, time : %f"), Distance, time);
	GetWorldTimerManager().SetTimer(characterTimer, this, &AStrategistPlayerController::myTimer, time + 0.1, false);
}

void AStrategistPlayerController::MoveToTouchLocation(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	FVector2D ScreenSpaceLocation(Location);

	// Trace to see what is under the touch location
	FHitResult HitResult;

	GetHitResultAtScreenPosition(ScreenSpaceLocation, CurrentClickTraceChannel, true, HitResult);
	if (HitResult.bBlockingHit)
	{
		// We hit something, move there
		SetNewMoveDestination(HitResult.ImpactPoint);
	}
}

void AStrategistPlayerController::SetNewMoveDestination(const FVector DestLocation)
{
	//if click goal point 
	//start timer

}

void AStrategistPlayerController::OnSetDestinationPressed()
{
	// set flag to keep updating destination until released
	bMoveToMouseCursor = true;

	FHitResult Hit;
	GetHitResultUnderCursor(ECC_Visibility, false, Hit);
	//GetHitResultUnderCursorForObjects

	if (getGameType() == EGameState::EEditMode)
	{
		for (int i = 1; i < checkpoints.Num() - 1; i++)
		{
			//get checkpoint
			ACheckPoint* checkpoint = checkpoints[i];

			//if length between cursor point and checkpoint is shorter than distance
			if (FVector::Dist(checkpoint->GetActorLocation(), Hit.Location) <= distance)
			{
				//return that checkpoint
				UE_LOG(PlayerController, Log, TEXT("Found checkpoint"));
				pickedCheckPoint = checkpoint;
				pickedIndex = i;
				dragMode = true;
				return;
			}
		}
	}
}

void AStrategistPlayerController::OnSetDestinationReleased()
{
	if (GetPawn()->GetClass() == BP_Center)
	{
		// clear flag to indicate we should stop updating the destination
		bMoveToMouseCursor = false;

		FHitResult Hit;
		GetHitResultUnderCursor(ECC_Visibility, false, Hit);

		int currentCheckLimit = ((AStrategistGameMode*)GetWorld()->GetAuthGameMode())->getCurrentCheckLimit();
		if (Hit.bBlockingHit && !dragMode && currentCheckLimit > 0)
		{
			UE_LOG(PlayerController, Log, TEXT("Picked Location : X : %f, Y : %f, Z : %f")
				, Hit.ImpactPoint.X, Hit.ImpactPoint.Y, Hit.ImpactPoint.Z);

			//	Spawn cube indicator
			AddCheckPoint(Hit.Location, checkpointBP, checkpoints.Num() - 1);
			MakeLine(checkpoints.Num() - 2, EDIT_STATES::EADD);

			((AStrategistGameMode*)GetWorld()->GetAuthGameMode())->subCurrentCheckLimit();
		}

		dragMode = false;
	}
}

void AStrategistPlayerController::OnSimulationStarted()
{

	if (!isSimulated && GetPawn()->GetClass() == BP_Center)
	{
		Morph();
		myTimer();
	}
}

void AStrategistPlayerController::OnSimulationRestarted()
{
	if (((getGameType() == EGameState::ESimulationFailed) || 
		(getGameType() == EGameState::ESimulationSuccessed)) && !isSimulated)
	{
		setGameType(EGameState::EEditMode);
		APawn* const MyPawn = GetPawn();
		((AStrategistCharacter*)MyPawn)->RestartSimulation_BP();

		((AStrategistGameMode*)GetWorld()->GetAuthGameMode())->resetCurrentCheckLimit();

		Morph();
	}
}

void AStrategistPlayerController::OnDeleteCheckPoint()
{
	if (GetPawn()->GetClass() == BP_Center)
	{
		FHitResult Hit;
		GetHitResultUnderCursor(ECC_Visibility, false, Hit);

		for (int i = 1; i < checkpoints.Num()-1; i++)
		{
			//get checkpoint
			ACheckPoint* checkpoint = checkpoints[i];

			//if length between cursor point and checkpoint is shorter than distance
			if (FVector::Dist(checkpoint->GetActorLocation(), Hit.Location) <= distance)
			{
				//delete that checkpoint
				checkpoints.Remove(checkpoint);
				checkpoint->Destroy();
				UE_LOG(PlayerController, Log, TEXT("Delete checkpoint"));

				MakeLine(i, EDIT_STATES::EDELETE);

				((AStrategistGameMode*)GetWorld()->GetAuthGameMode())->addCurrentCheckLimit();
				return;
			}
		}
	}
}

void AStrategistPlayerController::MakeLine(int indexOfCheckPoint, EDIT_STATES flag)
{
	//delete left and right sphere
	if (checkpoints.Num() > 2 || flag == EDIT_STATES::EDELETE)
	{
		//delete
		for (int i = 0; i < sphereArray[indexOfCheckPoint - 1].Num(); i++)
		{
			AActor* sphere = sphereArray[indexOfCheckPoint - 1][i];
			sphere->Destroy();
		}
		if (sphereArray.Num() > 1 && (flag == EDIT_STATES::EDRAG || flag == EDIT_STATES::EDELETE))
		{
			for (int i = 0; i < sphereArray[indexOfCheckPoint].Num(); i++)
			{
				AActor* sphere = sphereArray[indexOfCheckPoint][i];
				sphere->Destroy();
			}
			sphereArray.RemoveAt(indexOfCheckPoint - 1);
		}
		sphereArray.RemoveAt(indexOfCheckPoint - 1);
		UE_LOG(PlayerController, Log, TEXT("delete"));
	}

	//get two checkpoint's location
	FVector start, end;
	start = checkpoints[indexOfCheckPoint - 1]->GetActorLocation();
	end = checkpoints[indexOfCheckPoint]->GetActorLocation();

	//get distance between two checkpoints
	float distance = FVector::Dist(start, end);

	//get direction from start to end
	FVector dir = end - start;
	dir.Normalize();

	TArray<AActor*> spheres;

	for (int j = 0; j < (int)(distance / 100); j++)
	{
		FVector pos = start + (dir * 100) * j;
		FRotator* rot = new FRotator(0, 0, 0);

		//spawn sphere
		UWorld* const World = GetWorld();
		AActor* sphere = World->SpawnActor<AActor>(lineSphereBP, pos, *rot);
		spheres.Add(sphere);
	}
	sphereArray.Insert(spheres, indexOfCheckPoint - 1);

	if (checkpoints.Num() > 2 && flag != EDIT_STATES::EDELETE)
	{
		//get two checkpoint's location
		FVector start, end;
		start = checkpoints[indexOfCheckPoint]->GetActorLocation();
		end = checkpoints[indexOfCheckPoint + 1]->GetActorLocation();

		//get distance between two checkpoints
		float distance = FVector::Dist(start, end);

		//get direction from start to end
		FVector dir = end - start;
		dir.Normalize();

		TArray<AActor*> spheres;

		for (int j = 0; j < (int)(distance / 100); j++)
		{
			FVector pos = start + (dir * 100) * j;
			FRotator* rot = new FRotator(0, 0, 0);

			//spawn sphere
			UWorld* const World = GetWorld();
			AActor* sphere = World->SpawnActor<AActor>(lineSphereBP, pos, *rot);
			spheres.Add(sphere);
		}

		sphereArray.Insert(spheres, indexOfCheckPoint);
	}

	UE_LOG(PlayerController, Log, TEXT("Make line"));
}

void AStrategistPlayerController::AddCheckPoint(FVector Location, TSubclassOf<class ACheckPoint> checkpoint_bp, int index)
{
	//	Spawn cube indicator
	UWorld* const World = GetWorld();
	if (World)
	{
		//	Set the spawn parameters
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = Instigator;

		// Set spawn location, rotation
		FVector SpawnLocation = Location;
		FRotator SpawnRotation(0, 0, 0);

		ACheckPoint* checkpoint = World->SpawnActor<ACheckPoint>(checkpoint_bp,
			SpawnLocation, SpawnRotation, SpawnParams);

		checkpoints.Insert(checkpoint, index);
	}
}
void AStrategistPlayerController::setGameType(EGameState newType) {
	gameType = newType;
}
EGameState AStrategistPlayerController::getGameType() {
	return gameType;
}

void AStrategistPlayerController::SetNewLevel()
{
	if (getGameType() == EGameState::ESimulationSuccessed && !isSimulated)
	{
		((AStrategistGameMode*)GetWorld()->GetAuthGameMode())->ChangeLevel();
	}
}