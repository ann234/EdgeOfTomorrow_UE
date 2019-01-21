// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameMode.h"
#include "MainMenuGameMode.generated.h"

/**
 * 
 */
UCLASS()
class STRATEGIST_API AMainMenuGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "myFunction")
		int32 getCurrentLevel();
	UFUNCTION(BlueprintCallable, Category = "myFunction")
		void setCurrentLevel(int32 _currentlevel);
	
	
};
