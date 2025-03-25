#include "TetrisBlock.h"
#include "GameFramework/PlayerController.h"

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
    FVector NewLocation = GetActorLocation();
    NewLocation.Z -= 100.0f; // 아래로 이동
    SetActorLocation(NewLocation);

    UE_LOG(LogTemp, Warning, TEXT("Block moved down to: %s"), *NewLocation.ToString());
}

void ATetrisBlock::Rotate()
{
    FRotator NewRotation = GetActorRotation();
    NewRotation.Yaw += 90.0f; // 90도 회전
    SetActorRotation(NewRotation);

    UE_LOG(LogTemp, Warning, TEXT("Block rotated to: %s"), *NewRotation.ToString());
}

void ATetrisBlock::MoveLeft()
{
    FVector NewLocation = GetActorLocation();
    NewLocation.X -= 100.0f; // 왼쪽으로 이동
    SetActorLocation(NewLocation);

    UE_LOG(LogTemp, Warning, TEXT("Block moved left to: %s"), *NewLocation.ToString());
}

void ATetrisBlock::MoveRight()
{
    FVector NewLocation = GetActorLocation();
    NewLocation.X += 100.0f; // 오른쪽으로 이동
    SetActorLocation(NewLocation);

    UE_LOG(LogTemp, Warning, TEXT("Block moved right to: %s"), *NewLocation.ToString());
}