// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/PlayerController.h"
#include "StrategistGameMode.h"
#include "StrategistPlayerController.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(PlayerController, Log, All);

UENUM(BlueprintType)
enum class EDIT_STATES {
	EADD,
	EDRAG,
	EDELETE
};

UCLASS()
class AStrategistPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AStrategistPlayerController();

	virtual void BeginPlay() override;

	//Function to change shape
	UFUNCTION(BlueprintCallable, Category = "myFunction")
	void Morph();
	UFUNCTION(BlueprintCallable, Category = "myFunction")
	void OnSimulationStarted();
	UFUNCTION(BlueprintCallable, Category = "myFunction")
	void resetSimulationParams();
	UFUNCTION(BlueprintCallable, Category = "myFunction")
	void setGameType(EGameState newType);
	UFUNCTION(BlueprintCallable, Category = "myFunction")
	EGameState getGameType();

	UFUNCTION(BlueprintCallable, Category = "myFunction")
		void SetNewLevel();
	UFUNCTION(BlueprintCallable, Category = "myFunction")
		void OnSimulationRestarted();

	TSubclassOf<APawn> BP_Player;
	TSubclassOf<APawn> BP_Center;

protected:
	/**The pickup to spawn*/
	TSubclassOf<class ACheckPoint>	WhatToSpawn;

	/** True if the controlled character should navigate to the mouse cursor. */
	uint32 bMoveToMouseCursor : 1;

	// Begin PlayerController interface
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;
	// End PlayerController interface

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Navigate player to the current mouse cursor location. */
	void MoveToMouseCursor();

	/** Navigate player to the current touch location. */
	void MoveToTouchLocation(const ETouchIndex::Type FingerIndex, const FVector Location);

	/** Navigate player to the given world location. */
	void SetNewMoveDestination(const FVector DestLocation);

	/** Input handlers for SetDestination action. */
	void OnSetDestinationPressed();
	void OnSetDestinationReleased();
	void OnDeleteCheckPoint();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = myHandler, meta = (AllowPrivateAccess = "true"))
	FTimerHandle characterTimer;

	TArray<FVector*> vectorList;
	int listIter = 0;
	const int characterZ = 149.149994;

	//Store character's start location 
	const FVector initCharacterPos;

	//checkpoint blueprint for making checkpoint dynaically
	TSubclassOf<class ACheckPoint> checkpointBP;
	TSubclassOf<class ACheckPoint> checkpointBP_startPoint;
	TSubclassOf<class ACheckPoint> checkpointBP_endPoint;

	//sphere blueprint for drawing line
	TSubclassOf<class AActor> lineSphereBP;
	//store sphere
	TArray<TArray<AActor*>> sphereArray;

	TArray<ACheckPoint*> checkpoints;
	void AddCheckPoint(FVector Location, TSubclassOf<class ACheckPoint> type, int index);
	
	//toggle simulation
	bool isSimulated = false;

	//true if cube exists in mouse left click's point
	bool dragMode = false;

	//store picked object for dragging
	ACheckPoint* pickedCheckPoint;
	int pickedIndex;

	float distance = 200.0f;

	//Make and refresh lineSphere when checkpoint is add or remove
	void MakeLine(int indexOfCheckPoint, EDIT_STATES flag);

	void myTimer();

	//gamemode에게 현재 상태 넘겨주기위한 변수. 0 : edit, 1 : 시뮬중, 2 : 시뮬실패, 3: 시뮬성공
	EGameState gameType;

	int currentLevel = 1;
	FString myGameName = "FirstStage";
};


