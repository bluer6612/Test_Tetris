#include "TetrisBoard.h"
#include "Blocks/TetrisBlock.h" // TetrisBlock 클래스의 경로
#include "Engine/World.h"       // GetWorld() 함수 사용
#include "UObject/ConstructorHelpers.h" // ConstructorHelpers 사용 시 필요
#include "GameFramework/Actor.h" // AActor 관련 기능
#include "Engine/Engine.h"       // UE_LOG 사용 시 필요

ATetrisBoard::ATetrisBoard()
{
    PrimaryActorTick.bCanEverTick = true;

    // 기본적으로 사용할 블록 클래스를 설정 (필요 시)
    static ConstructorHelpers::FClassFinder<ATetrisBlock> BlockBPClass(TEXT("/Game/Blueprints/BP_TetrisBlock"));
    if (BlockBPClass.Class != nullptr)
    {
        BlockClass = BlockBPClass.Class;
    }
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
        else
        {
            UE_LOG(LogTemp, Error, TEXT("BlockClass is not set!"));
        }
    }
}

void ATetrisBoard::ClearFullRows()
{
    UE_LOG(LogTemp, Warning, TEXT("Full rows cleared"));
}