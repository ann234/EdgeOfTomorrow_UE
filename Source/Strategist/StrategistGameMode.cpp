// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "Strategist.h"
#include "StrategistGameMode.h"
#include "StrategistPlayerController.h"
#include "StrategistCharacter.h"
#include "Blueprint/UserWidget.h"

#include "StrategistGameInstance.h"

AStrategistGameMode::AStrategistGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = AStrategistPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDownCPP/Blueprints/BP_Center"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	PrimaryActorTick.bCanEverTick = true;

}

void AStrategistGameMode::BeginPlay() {
	Super::BeginPlay();
	SetCurrentState(EGameState::EEditMode);

	CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), HUDWidget);
	CurrentWidget->AddToViewport();

	resetCurrentCheckLimit();
}


void AStrategistGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

EGameState AStrategistGameMode::GetCurrentState() {
	return currentState;
}

void AStrategistGameMode::SetCurrentState(EGameState newState) {
	currentState = newState;
}

void AStrategistGameMode::ChangeLevel()
{
	UStrategistGameInstance* gameInstance = Cast<UStrategistGameInstance>(GetGameInstance());
	(gameInstance->currentLevel)++;
	if (gameInstance->currentLevel > 3)
		gameInstance->currentLevel = 1;

	int currentLevel = gameInstance->currentLevel;

	FString NewLevel = myGameName + FString::FromInt(currentLevel);

	UE_LOG(PlayerController, Log, TEXT("currentlevel: %d"), currentLevel);

	currentState = EGameState::EEditMode;
	UGameplayStatics::OpenLevel(GetWorld(), FName(*NewLevel));
	UE_LOG(PlayerController, Log, TEXT("%s"), *NewLevel);
}

int32 AStrategistGameMode::getCurrentLevel()
{
	UStrategistGameInstance* gameInstance = Cast<UStrategistGameInstance>(GetGameInstance());
	return gameInstance->currentLevel;
}

void AStrategistGameMode::setCurrentLevel(int32 _currentLevel)
{
	UStrategistGameInstance* gameInstance = Cast<UStrategistGameInstance>(GetGameInstance());
	gameInstance->currentLevel = _currentLevel;
}

void AStrategistGameMode::resetCurrentCheckLimit()
{
	UStrategistGameInstance* gameInstance = Cast<UStrategistGameInstance>(GetGameInstance());
	currentCheckLimit = gameInstance->checkLimit[gameInstance->currentLevel - 1];
}

int AStrategistGameMode::getCurrentCheckLimit()
{
	return currentCheckLimit;
}

void AStrategistGameMode::addCurrentCheckLimit()
{
	currentCheckLimit++;
}

void AStrategistGameMode::subCurrentCheckLimit()
{
	currentCheckLimit--;
}