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

    if (bIsGameOver)
    {
        UE_LOG(LogTemp, Warning, TEXT("Game is over. No further actions."));
        return;
    }

    TimeSinceLastFall += DeltaTime;

    if (ActiveBlock && TimeSinceLastFall >= BlockFallInterval)
    {
        FVector NewLocation = ActiveBlock->GetActorLocation();
        NewLocation.Z -= 100.0f; // 아래로 이동
        ActiveBlock->SetActorLocation(NewLocation);

        TimeSinceLastFall = 0.0f; // 시간 초기화

        if (HasCollision(NewLocation))
        {
            ActiveBlock->SetActorLocation(NewLocation + FVector(0, 0, 100.0f)); // 원래 위치로 복구
            ClearFullRows(); // 줄 제거
            SpawnBlock();
        }
    }
}

void ATetrisBoard::SpawnBlock()
{
    if (GetWorld())
    {
        FVector SpawnLocation = FVector(0.0f, 0.0f, 500.0f); // 보드 위쪽
        FRotator SpawnRotation = FRotator::ZeroRotator;

        if (BlockClass)
        {
            ActiveBlock = GetWorld()->SpawnActor<ATetrisBlock>(BlockClass, SpawnLocation, SpawnRotation);

            if (ActiveBlock)
            {
                TArray<FVector> BlockShape;
                int RandomShape = FMath::RandRange(0, 2); // 0~2 사이의 랜덤 값
                switch (RandomShape)
                {
                case 0:
                    BlockShape = IBlock;
                    break;
                case 1:
                    BlockShape = TBlock;
                    break;
                case 2:
                    BlockShape = LBlock;
                    break;
                }

                ActiveBlock->InitializeBlock(BlockShape);

                // 게임 오버 감지
                if (HasCollision(ActiveBlock->GetActorLocation()))
                {
                    bIsGameOver = true;
                    UE_LOG(LogTemp, Error, TEXT("Game Over!"));
                }

                UE_LOG(LogTemp, Warning, TEXT("New block spawned at location: %s"), *SpawnLocation.ToString());
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to spawn ActiveBlock!"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("BlockClass is not set!"));
        }
    }
}

bool ATetrisBoard::HasCollision(const FVector& Location)
{
    // 보드 경계 충돌 감지
    if (Location.X < 0.0f || Location.X >= BoardWidth * 100.0f || Location.Z <= 0.0f)
    {
        return true;
    }

    // TODO: 다른 블록과의 충돌 감지 로직 추가
    // 예: Board 배열을 사용하여 충돌 여부 확인
    int XIndex = FMath::FloorToInt(Location.X / 100.0f);
    int ZIndex = FMath::FloorToInt(Location.Z / 100.0f);

    if (XIndex >= 0 && XIndex < BoardWidth && ZIndex >= 0 && ZIndex < BoardHeight)
    {
        if (Board[XIndex][ZIndex])
        {
            return true;
        }
    }

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