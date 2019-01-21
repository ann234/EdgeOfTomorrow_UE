// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/GameInstance.h"
#include "StrategistGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class STRATEGIST_API UStrategistGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UStrategistGameInstance(const FObjectInitializer& ObjectInitializer);

	int32 currentLevel = 1;

	TArray<int> checkLimit = { 3, 3, 4 };
};
