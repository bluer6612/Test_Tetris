#include "TetrisGameModeBase.h"
#include "Engine/World.h"

ATetrisGameModeBase::ATetrisGameModeBase()
{
    // Set default values
}

void ATetrisGameModeBase::BeginPlay()
{
    Super::BeginPlay();
    StartGame();
}

void ATetrisGameModeBase::StartGame()
{
    UE_LOG(LogTemp, Warning, TEXT("Tetris Game Started!"));
}

void ATetrisGameModeBase::EndGame()
{
    UE_LOG(LogTemp, Warning, TEXT("Tetris Game Ended!"));
}