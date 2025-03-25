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

    if (ActiveBlock)
    {
        FVector NewLocation = ActiveBlock->GetActorLocation();
        NewLocation.Z -= 100.0f * DeltaTime; // 아래로 이동
        ActiveBlock->SetActorLocation(NewLocation);

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
                int RandomShape = FMath::RandRange(0, 2);
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
            }
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
    for (int y = 0; y < BoardHeight; ++y)
    {
        bool bIsRowFull = true;

        // 현재 줄이 가득 찼는지 확인
        for (int x = 0; x < BoardWidth; ++x)
        {
            if (!Board[x][y])
            {
                bIsRowFull = false;
                break;
            }
        }

        // 가득 찬 줄 제거
        if (bIsRowFull)
        {
            // 현재 줄 제거
            for (int x = 0; x < BoardWidth; ++x)
            {
                Board[x][y] = false;
            }

            // 위의 블록들을 아래로 이동
            for (int yy = y; yy < BoardHeight - 1; ++yy)
            {
                for (int x = 0; x < BoardWidth; ++x)
                {
                    Board[x][yy] = Board[x][yy + 1];
                }
            }

            // 맨 위 줄 초기화
            for (int x = 0; x < BoardWidth; ++x)
            {
                Board[x][BoardHeight - 1] = false;
            }

            UE_LOG(LogTemp, Warning, TEXT("Row %d cleared!"), y);

            // 한 줄 제거 후 다시 확인
            --y;
        }
    }
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

        // 회전 후 충돌 감지
        ActiveBlock->SetActorRotation(NewRotation);
        if (HasCollision(ActiveBlock->GetActorLocation()))
        {
            // 충돌이 발생하면 회전을 취소
            NewRotation.Yaw -= 90.0f;
            ActiveBlock->SetActorRotation(NewRotation);
        }
    }
}