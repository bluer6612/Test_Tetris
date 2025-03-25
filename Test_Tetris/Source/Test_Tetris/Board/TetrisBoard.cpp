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

    // 입력 바인딩 설정
    EnableInput(GetWorld()->GetFirstPlayerController());

    InputComponent->BindAction("MoveLeft", IE_Pressed, this, &ATetrisBoard::MoveLeft);
    InputComponent->BindAction("MoveRight", IE_Pressed, this, &ATetrisBoard::MoveRight);
    InputComponent->BindAction("MoveDown", IE_Pressed, this, &ATetrisBoard::MoveDown);
    InputComponent->BindAction("Rotate", IE_Pressed, this, &ATetrisBoard::RotateBlock);

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
        //NewLocation.Z -= 10.0f; // 아래로 이동
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
    if (GetWorld())
    {
        FVector SpawnLocation = FVector(0.0f, 0.0f, BoardHeight * 100.0f); // 보드 위쪽
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
    int XIndex = FMath::FloorToInt(Location.X / 100.0f);
    int ZIndex = FMath::FloorToInt(Location.Z / 100.0f);

    UE_LOG(LogTemp, Warning, TEXT("Checking collision at X: %d, Z: %d"), XIndex, ZIndex);

    if (Location.X < 0.0f || Location.X >= BoardWidth * 100.0f || Location.Z <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Collision with board boundary detected."));
        return true;
    }

    if (XIndex >= 0 && XIndex < BoardWidth && ZIndex >= 0 && ZIndex < BoardHeight)
    {
        if (Board[XIndex][ZIndex])
        {
            UE_LOG(LogTemp, Warning, TEXT("Collision with existing block detected."));
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
        FVector Offset = FVector(-100.0f, 0.0f, 0.0f); // 왼쪽으로 이동
        if (!HasCollision(ActiveBlock->GetActorLocation() + Offset))
        {
            ActiveBlock->Move(Offset);
        }
    }
}

void ATetrisBoard::MoveRight()
{
    if (ActiveBlock)
    {
        FVector Offset = FVector(100.0f, 0.0f, 0.0f); // 오른쪽으로 이동
        if (!HasCollision(ActiveBlock->GetActorLocation() + Offset))
        {
            ActiveBlock->Move(Offset);
        }
    }
}

void ATetrisBoard::MoveDown()
{
    if (ActiveBlock)
    {
        FVector Offset = FVector(0.0f, 0.0f, -100.0f); // 아래로 이동
        if (!HasCollision(ActiveBlock->GetActorLocation() + Offset))
        {
            ActiveBlock->Move(Offset);
        }
        else
        {
            // 충돌 시 블록 고정 및 새로운 블록 생성
            ClearFullRows();
            SpawnBlock();
        }
    }
}

void ATetrisBoard::RotateBlock()
{
    if (ActiveBlock)
    {
        ActiveBlock->Rotate();
        if (HasCollision(ActiveBlock->GetActorLocation()))
        {
            // 충돌 시 회전 취소
            ActiveBlock->Rotate(); // 다시 회전하여 원래 상태로 복구
        }
    }
}