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

    // 블록이 일정 간격으로만 내려가도록 시간 누적
    TimeSinceLastFall += DeltaTime;

    if (ActiveBlock && TimeSinceLastFall >= BlockFallInterval)
    {
        FVector NewLocation = ActiveBlock->GetActorLocation();
        NewLocation.Z -= 100.0f; // 아래로 이동
        ActiveBlock->SetActorLocation(NewLocation);

        TimeSinceLastFall = 0.0f; // 시간 초기화

        // 충돌 감지
        if (HasCollision(NewLocation))
        {
            ActiveBlock->SetActorLocation(NewLocation + FVector(0, 0, 100.0f)); // 원래 위치로 복구
            ClearFullRows(); // 줄 제거
            SpawnBlock(); // 새로운 블록 생성
        }
    }
}

void ATetrisBoard::SpawnBlock()
{
    if (ActiveBlock)
    {
        // 현재 블록의 위치를 보드 상태에 반영
        for (UStaticMeshComponent* Mesh : ActiveBlock->GetBlockMeshes())
        {
            FVector BlockLocation = Mesh->GetComponentLocation();
            int XIndex = FMath::FloorToInt(BlockLocation.X / 100.0f);
            int ZIndex = FMath::FloorToInt(BlockLocation.Z / 100.0f);

            if (XIndex >= 0 && XIndex < BoardWidth && ZIndex >= 0 && ZIndex < BoardHeight)
            {
                Board[XIndex][ZIndex] = true; // 해당 위치를 차지했다고 표시
            }
        }
    }

    // 새로운 블록 생성
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

bool ATetrisBoard::HasCollision(const FVector& Location)
{
    // 보드 경계 충돌 감지
    if (Location.X < 0.0f || Location.X >= BoardWidth * 100.0f || Location.Z <= 0.0f)
    {
        return true;
    }

    // 보드 상태를 기반으로 충돌 감지
    int XIndex = FMath::FloorToInt(Location.X / 100.0f);
    int ZIndex = FMath::FloorToInt(Location.Z / 100.0f);

    if (XIndex >= 0 && XIndex < BoardWidth && ZIndex >= 0 && ZIndex < BoardHeight)
    {
        if (Board[XIndex][ZIndex]) // 해당 위치에 블록이 이미 존재하면 충돌
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