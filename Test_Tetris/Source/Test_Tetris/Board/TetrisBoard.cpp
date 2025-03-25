#include "TetrisBoard.h"
#include "../Blocks/TetrisBlock.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraActor.h"

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

    // 플레이어 컨트롤러 가져오기
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();

    if (PlayerController)
    {
        // 카메라 액터 생성
        ACameraActor* CameraActor = GetWorld()->SpawnActor<ACameraActor>();
        if (CameraActor)
        {
            // 카메라 위치와 방향 설정
            CameraActor->SetActorLocation(FVector(-2500.0f, 0.0f, 1000.0f)); // 블록 정면을 더 넓게 볼 수 있는 위치
            CameraActor->SetActorRotation(FRotator(0.0f, 0.0f, 0.0f));    // 블록을 향하도록 회전

            // 카메라를 뷰 타겟으로 설정
            PlayerController->SetViewTarget(CameraActor);
        }
    }

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
    if (ActiveBlock)
    {
        // 현재 블록의 위치를 보드 상태에 반영
        for (UStaticMeshComponent* Mesh : ActiveBlock->GetBlockMeshes())
        {
            FVector BlockLocation = Mesh->GetComponentLocation();
            int YIndex = FMath::FloorToInt(BlockLocation.Y / 100.0f);
            int ZIndex = FMath::FloorToInt(BlockLocation.Z / 100.0f);

            if (YIndex >= 0 && YIndex < BoardWidth && ZIndex >= 0 && ZIndex < BoardHeight)
            {
                Board[YIndex][ZIndex] = true; // 해당 위치를 차지했다고 표시
            }
        }
    }

    // 새로운 블록 생성
    FVector SpawnLocation = FVector(0.0f, BoardWidth / 2 * 100.0f, BoardHeight * 100.0f); // 보드 중앙 위쪽
    FRotator SpawnRotation = FRotator::ZeroRotator; // 블록을 세운 상태로 생성

    if (BlockClass)
    {
        ActiveBlock = GetWorld()->SpawnActor<ATetrisBlock>(BlockClass, SpawnLocation, SpawnRotation);

        if (ActiveBlock)
        {
            TArray<FVector> BlockShape;
            int RandomShape = FMath::RandRange(0, 6); // 0~6 사이의 랜덤 값
            switch (RandomShape)
            {
            case 0:
                BlockShape = IBlock;
                break;
            case 1:
                BlockShape = OBlock;
                break;
            case 2:
                BlockShape = TBlock;
                break;
            case 3:
                BlockShape = LBlock;
                break;
            case 4:
                BlockShape = JBlock;
                break;
            case 5:
                BlockShape = SBlock;
                break;
            case 6:
                BlockShape = ZBlock;
                break;
            }

            // 블록 초기화
            ActiveBlock->InitializeBlock(BlockShape);

            // 게임 오버 감지
            if (HasCollision(ActiveBlock->GetActorLocation()))
            {
                bIsGameOver = true;
                UE_LOG(LogTemp, Error, TEXT("Game Over! Block spawned in a collision state."));
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("New block spawned at location: %s"), *SpawnLocation.ToString());
            }
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
    if (!ActiveBlock)
    {
        return false; // ActiveBlock이 없으면 충돌 없음
    }

    for (UStaticMeshComponent* Mesh : ActiveBlock->GetBlockMeshes())
    {
        // 블록의 새로운 위치 계산
        FVector BlockLocation = Mesh->GetComponentLocation() + (Location - ActiveBlock->GetActorLocation());

        // 보드 경계 충돌 감지
        if (BlockLocation.Y < 0.0f || BlockLocation.Y >= BoardWidth * 100.0f || BlockLocation.Z <= 0.0f)
        {
            return true; // 보드 경계를 벗어남
        }

        // 다른 블록과의 충돌 감지
        int YIndex = FMath::FloorToInt(BlockLocation.Y / 100.0f);
        int ZIndex = FMath::FloorToInt(BlockLocation.Z / 100.0f);

        if (YIndex >= 0 && YIndex < BoardWidth && ZIndex >= 0 && ZIndex < BoardHeight)
        {
            if (Board[YIndex][ZIndex]) // 해당 위치에 블록이 이미 존재하면 충돌
            {
                return true;
            }
        }
    }

    return false; // 충돌 없음
}

void ATetrisBoard::ClearFullRows()
{
    UE_LOG(LogTemp, Warning, TEXT("Full rows cleared"));
}

void ATetrisBoard::MoveLeft()
{
    if (ActiveBlock)
    {
        FVector Offset = FVector(0.0f, -100.0f, 0.0f); // Y축 기준으로 왼쪽으로 이동
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
        FVector Offset = FVector(0.0f, 100.0f, 0.0f); // Y축 기준으로 오른쪽으로 이동
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
        FVector Offset = FVector(0.0f, 0.0f, -100.0f); // Z축 기준으로 아래로 이동
        FVector NewLocation = ActiveBlock->GetActorLocation() + Offset;

        // 충돌 감지
        if (!HasCollision(NewLocation))
        {
            ActiveBlock->Move(Offset);
        }
        else
        {
            // 충돌 시 블록 고정 및 새로운 블록 생성
            for (UStaticMeshComponent* Mesh : ActiveBlock->GetBlockMeshes())
            {
                FVector BlockLocation = Mesh->GetComponentLocation();
                int YIndex = FMath::FloorToInt(BlockLocation.Y / 100.0f);
                int ZIndex = FMath::FloorToInt(BlockLocation.Z / 100.0f);

                if (YIndex >= 0 && YIndex < BoardWidth && ZIndex >= 0 && ZIndex < BoardHeight)
                {
                    Board[YIndex][ZIndex] = true; // 해당 위치를 차지했다고 표시
                }
            }

            ClearFullRows(); // 줄 제거
            SpawnBlock(); // 새로운 블록 생성
        }
    }
}

void ATetrisBoard::RotateBlock()
{
    if (ActiveBlock)
    {
        // 기존 위치 저장 (충돌 시 복구를 위해)
        TArray<FVector> OriginalLocations;
        for (UStaticMeshComponent* Mesh : ActiveBlock->GetBlockMeshes())
        {
            OriginalLocations.Add(Mesh->GetRelativeLocation());
        }

        // 블록의 중심 계산
        FVector BlockCenter = FVector::ZeroVector;
        for (UStaticMeshComponent* Mesh : ActiveBlock->GetBlockMeshes())
        {
            BlockCenter += Mesh->GetRelativeLocation();
        }
        BlockCenter /= ActiveBlock->GetBlockMeshes().Num(); // 중심점 계산

        // 각 큐브의 상대 위치를 기준으로 90도 회전
        for (UStaticMeshComponent* Mesh : ActiveBlock->GetBlockMeshes())
        {
            FVector RelativeLocation = Mesh->GetRelativeLocation() - BlockCenter;

            // 2D 회전 변환 (Z축은 고정)
            float NewX = -RelativeLocation.Y;
            float NewY = RelativeLocation.X;

            // 새로운 위치 설정 (중심점 기준으로 이동)
            Mesh->SetRelativeLocation(FVector(NewX, NewY, RelativeLocation.Z) + BlockCenter);
        }

        // 충돌 감지
        if (HasCollision(ActiveBlock->GetActorLocation()))
        {
            // 충돌 시 원래 위치로 복구
            for (int32 i = 0; i < ActiveBlock->GetBlockMeshes().Num(); i++)
            {
                ActiveBlock->GetBlockMeshes()[i]->SetRelativeLocation(OriginalLocations[i]);
            }

            UE_LOG(LogTemp, Warning, TEXT("Rotation canceled due to collision."));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Block rotated successfully."));
        }
    }
}