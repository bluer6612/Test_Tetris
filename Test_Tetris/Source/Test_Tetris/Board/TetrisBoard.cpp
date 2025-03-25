#include "TetrisBoard.h"
#include "../Blocks/TetrisBlock.h" // TetrisBlock 클래스의 경로
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

    // 현재 활성 블록이 있는 경우 아래로 이동
    if (ActiveBlock)
    {
        FVector NewLocation = ActiveBlock->GetActorLocation();
        NewLocation.Z -= 100.0f * DeltaTime; // DeltaTime을 곱해 부드럽게 이동
        ActiveBlock->SetActorLocation(NewLocation);

        // 충돌 감지 (예: 바닥에 닿았는지 확인)
        if (HasCollision(NewLocation))
        {
            // 블록을 멈추고 새로운 블록 스폰
            ActiveBlock->SetActorLocation(NewLocation + FVector(0, 0, 100.0f)); // 원래 위치로 복구
            SpawnBlock();
        }
    }
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

bool ATetrisBoard::HasCollision(const FVector& Location)
{
    // 간단한 충돌 감지: Z 좌표가 0 이하로 내려가면 충돌로 간주
    if (Location.Z <= 0.0f)
    {
        return true;
    }

    // TODO: 다른 블록과의 충돌 감지 로직 추가
    return false;
}

void ATetrisBoard::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAction("MoveLeft", IE_Pressed, this, &ATetrisBoard::MoveLeft);
    PlayerInputComponent->BindAction("MoveRight", IE_Pressed, this, &ATetrisBoard::MoveRight);
    PlayerInputComponent->BindAction("MoveDown", IE_Pressed, this, &ATetrisBoard::MoveDown);
    PlayerInputComponent->BindAction("Rotate", IE_Pressed, this, &ATetrisBoard::RotateBlock);
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