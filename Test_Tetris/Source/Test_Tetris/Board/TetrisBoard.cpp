#include "TetrisBoard.h"
#include "TetrisBlock.h"
#include "Engine/World.h"

ATetrisBoard::ATetrisBoard()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ATetrisBoard::BeginPlay()
{
    Super::BeginPlay();
    SpawnBlock();
}

void ATetrisBoard::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ATetrisBoard::SpawnBlock()
{
    if (GetWorld())
    {
        // 블록의 스폰 위치 설정
        FVector SpawnLocation = FVector(0.0f, 0.0f, 300.0f); // 보드 위쪽
        FRotator SpawnRotation = FRotator::ZeroRotator;

        // 블록 클래스가 설정되어 있는지 확인
        if (BlockClass)
        {
            // 블록 생성
            GetWorld()->SpawnActor<ATetrisBlock>(BlockClass, SpawnLocation, SpawnRotation);
            UE_LOG(LogTemp, Warning, TEXT("New block spawned at location: %s"), *SpawnLocation.ToString());
        }
    }
}

void ATetrisBoard::ClearFullRows()
{
    UE_LOG(LogTemp, Warning, TEXT("Full rows cleared"));
}