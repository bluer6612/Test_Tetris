#include "TetrisBoard.h"
#include "../Blocks/TetrisBlock.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"etrisBlock 클래스의 경로
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"rated.h"

ATetrisBoard::ATetrisBoard()
{lass ATetrisBlock;
    PrimaryActorTick.bCanEverTick = true;

    // 기본적으로 사용할 블록 클래스를 설정 (필요 시)
    static ConstructorHelpers::FClassFinder<ATetrisBlock> BlockBPClass(TEXT("/Game/Blueprints/BP_TetrisBlock"));
    if (BlockBPClass.Class != nullptr)
    {ENERATED_BODY()
        BlockClass = BlockBPClass.Class;
    }c:
}   ATetrisBoard();

void ATetrisBoard::BeginPlay()
{   virtual void BeginPlay() override;
    Super::BeginPlay();
    SpawnBlock();
}   virtual void Tick(float DeltaTime) override;

void ATetrisBoard::Tick(float DeltaTime)
{   void MoveLeft();
    Super::Tick(DeltaTime);
    void MoveDown();
    if (bIsGameOver)();
    {
        UE_LOG(LogTemp, Warning, TEXT("Game is over. No further actions."));
        return;lock(); // 블록 생성
    }oid ClearFullRows(); // 가득 찬 줄 제거
    bool HasCollision(const FVector& Location); // 충돌 감지 함수
    TimeSinceLastFall += DeltaTime;
/ 테트리스 보드 크기 정의
    if (ActiveBlock && TimeSinceLastFall >= BlockFallInterval)
    {0칸)
        FVector NewLocation = ActiveBlock->GetActorLocation();
        NewLocation.Z -= 100.0f; // 아래로 이동    // 보드 상태를 저장하는 2D 배열
        ActiveBlock->SetActorLocation(NewLocation);] = { false };

        TimeSinceLastFall = 0.0f; // 시간 초기화

        if (HasCollision(NewLocation))
        {Vector(100, 0, 0),
            ActiveBlock->SetActorLocation(NewLocation + FVector(0, 0, 100.0f)); // 원래 위치로 복구   FVector(-100, 0, 0),
            ClearFullRows(); // 줄 제거       FVector(-200, 0, 0)
            SpawnBlock();    };
        }
    }   TArray<FVector> TBlock = {
}0, 0),
   FVector(100, 0, 0),
void ATetrisBoard::SpawnBlock()
{
    if (GetWorld())    };
    {
        FVector SpawnLocation = FVector(0.0f, 0.0f, 500.0f); // 보드 위쪽y<FVector> LBlock = {
        FRotator SpawnRotation = FRotator::ZeroRotator;
        FVector(100, 0, 0),
        if (BlockClass)
        {or(-100, 0, 100)
            ActiveBlock = GetWorld()->SpawnActor<ATetrisBlock>(BlockClass, SpawnLocation, SpawnRotation);

            if (ActiveBlock)
            {tReadWrite, Category = "Tetris")
                // 랜덤한 블록 모양 선택TetrisBlock> BlockClass; // 블록 클래스 설정
                TArray<FVector> BlockShape;
                int RandomShape = FMath::RandRange(0, 2); // 0~2 사이의 랜덤 값
                switch (RandomShape)ock; // 현재 활성 블록
                {false; // 게임 오버 상태
                case 0:록이 내려오는 간격 (초 단위)
                    BlockShape = IBlock;l = 0.0f; // 마지막으로 블록이 내려온 이후 경과 시간
                    break;                case 1:                    BlockShape = TBlock;                    break;                case 2:                    BlockShape = LBlock;                    break;                }                ActiveBlock->InitializeBlock(BlockShape);                UE_LOG(LogTemp, Warning, TEXT("New block spawned at location: %s"), *SpawnLocation.ToString());            }            else            {                UE_LOG(LogTemp, Error, TEXT("Failed to spawn ActiveBlock!"));            }        }        else        {            UE_LOG(LogTemp, Error, TEXT("BlockClass is not set!"));        }    }}bool ATetrisBoard::HasCollision(const FVector& Location){    // 보드 경계 충돌 감지    if (Location.X < 0.0f || Location.X >= BoardWidth * 100.0f || Location.Z <= 0.0f)    {        return true;    }    // TODO: 다른 블록과의 충돌 감지 로직 추가    return false;}void ATetrisBoard::ClearFullRows(){    UE_LOG(LogTemp, Warning, TEXT("Full rows cleared"));}void ATetrisBoard::MoveLeft(){    if (ActiveBlock)    {        FVector NewLocation = ActiveBlock->GetActorLocation();        NewLocation.X -= 100.0f; // 왼쪽으로 이동        ActiveBlock->SetActorLocation(NewLocation);    }}void ATetrisBoard::MoveRight(){    if (ActiveBlock)    {        FVector NewLocation = ActiveBlock->GetActorLocation();        NewLocation.X += 100.0f; // 오른쪽으로 이동        ActiveBlock->SetActorLocation(NewLocation);    }}void ATetrisBoard::MoveDown(){    if (ActiveBlock)    {        FVector NewLocation = ActiveBlock->GetActorLocation();        NewLocation.Z -= 100.0f; // 아래로 이동        ActiveBlock->SetActorLocation(NewLocation);    }}void ATetrisBoard::RotateBlock(){    if (ActiveBlock)    {        FRotator NewRotation = ActiveBlock->GetActorRotation();
        NewRotation.Yaw += 90.0f; // 90도 회전
        ActiveBlock->SetActorRotation(NewRotation);
    }
}