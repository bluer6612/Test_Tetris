#include "TetrisBlock.h"

ATetrisBlock::ATetrisBlock()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ATetrisBlock::BeginPlay()
{
    Super::BeginPlay();
}

void ATetrisBlock::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ATetrisBlock::MoveDown()
{
    UE_LOG(LogTemp, Warning, TEXT("Block moved down"));
}

void ATetrisBlock::Rotate()
{
    UE_LOG(LogTemp, Warning, TEXT("Block rotated"));
}