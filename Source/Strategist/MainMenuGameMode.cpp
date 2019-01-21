// Fill out your copyright notice in the Description page of Project Settings.

#include "Strategist.h"
#include "MainMenuGameMode.h"

#include "StrategistGameInstance.h"

int32 AMainMenuGameMode::getCurrentLevel()
{
	UStrategistGameInstance* gameInstance = Cast<UStrategistGameInstance>(GetGameInstance());
	return gameInstance->currentLevel;
}

void AMainMenuGameMode::setCurrentLevel(int32 _currentLevel)
{
	UStrategistGameInstance* gameInstance = Cast<UStrategistGameInstance>(GetGameInstance());
	gameInstance->currentLevel = _currentLevel;
}


