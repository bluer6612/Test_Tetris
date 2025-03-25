#include "TetrisBoard.h"
#include "../Blocks/TetrisBlock.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"

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

    if (ActiveBlock)
    {
        FVector NewLocation = ActiveBlock->GetActorLocation();
        NewLocation.Z -= 100.0f * DeltaTime; // 아래로 이동
        ActiveBlock->SetActorLocation(NewLocation);

        if (HasCollision(NewLocation))
        {
            ActiveBlock->SetActorLocation(NewLocation + FVector(0, 0, 100.0f)); // 원래 위치로 복구
            SpawnBlock();
        }
    }
}

void ATetrisBoard::SpawnBlock()
{
    if (GetWorld())
    {
        FVector SpawnLocation = FVector(0.0f, 0.0f, 300.0f); // 보드 위쪽
        FRotator SpawnRotation = FRotator::ZeroRotator;

        if (BlockClass)
        {
            ActiveBlock = GetWorld()->SpawnActor<ATetrisBlock>(BlockClass, SpawnLocation, SpawnRotation);
            UE_LOG(LogTemp, Warning, TEXT("New block spawned at location: %s"), *SpawnLocation.ToString());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("BlockClass is not set!"));
        }
    }
}

bool ATetrisBoard::HasCollision(const FVector& Location)
{
    if (Location.Z <= 0.0f)
    {
        return true;
    }

    // TODO: 다른 블록과의 충돌 감지 로직 추가
    return false;
}

void ATetrisBoard::ClearFullRows()
{
    UE_LOG(LogTemp, Warning, TEXT("Full rows cleared"));
}

void ATetrisBoard::MoveLeft()
{
    if (ActiveBlock)
    {
        FVector NewLocation = ActiveBlock->GetActorLocation();
        NewLocation.X -= 100.0f; // 왼쪽으로 이동
        ActiveBlock->SetActorLocation(NewLocation);
    }
}

void ATetrisBoard::MoveRight()
{
    if (ActiveBlock)
    {
        FVector NewLocation = ActiveBlock->GetActorLocation();
        NewLocation.X += 100.0f; // 오른쪽으로 이동
        ActiveBlock->SetActorLocation(NewLocation);
    }
}

void ATetrisBoard::MoveDown()
{
    if (ActiveBlock)
    {
        FVector NewLocation = ActiveBlock->GetActorLocation();
        NewLocation.Z -= 100.0f; // 아래로 이동
        ActiveBlock->SetActorLocation(NewLocation);
    }
}

void ATetrisBoard::RotateBlock()
{
    if (ActiveBlock)
    {
        FRotator NewRotation = ActiveBlock->GetActorRotation();
        NewRotation.Yaw += 90.0f; // 90도 회전
        ActiveBlock->SetActorRotation(NewRotation);
    }
}