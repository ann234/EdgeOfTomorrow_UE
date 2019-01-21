// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/GameMode.h"

#include "StrategistGameMode.generated.h"

UENUM(BlueprintType)
enum class EGameState {
	EEditMode,
	ESimulating,
	ESimulationFailed,
	ESimulationSuccessed
};

UCLASS(minimalapi)
class AStrategistGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AStrategistGameMode();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;


	/* ���� ���� ���� ���Ͻ����ش�.*/
	UFUNCTION(BlueprintPure, Category = "myFunction")
	EGameState GetCurrentState();

	/*���ο� ���� ���·� ���ý����ش�.*/
	UFUNCTION(BlueprintCallable, Category = "myFunction")
	void SetCurrentState(EGameState newState);

	void ChangeLevel();

	UFUNCTION(BlueprintCallable, Category = "myFunction")
		int32 getCurrentLevel();
	UFUNCTION(BlueprintCallable, Category = "myFunction")
		void setCurrentLevel(int32 _currentlevel);

	UFUNCTION(BlueprintCallable, Category = "myFunction")
		int getCurrentCheckLimit();

	void resetCurrentCheckLimit();
	void addCurrentCheckLimit();
	void subCurrentCheckLimit();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "myProperty", Meta = (BlueprintProtected = "true"))
	TSubclassOf<class UUserWidget> HUDWidget;

	UPROPERTY()
	class UUserWidget* CurrentWidget;
private:
	EGameState currentState;

	int currentLevel = 1;
	FString myGameName = "FirstStage";

	int currentCheckLimit = 5;
};



