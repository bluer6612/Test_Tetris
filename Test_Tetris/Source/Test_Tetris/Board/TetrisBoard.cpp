#include "TetrisBoard.h"
#include "../Blocks/TetrisBlock.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraActor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"

ATetrisBoard::ATetrisBoard()
{
    PrimaryActorTick.bCanEverTick = true;

    // Root 컴포넌트 생성
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // 기본 스케일 설정
    DefaultScale = FVector(0.1f, 0.1f, 1.0f);

    // 기본적으로 사용할 블록 클래스를 설정
    static ConstructorHelpers::FClassFinder<ATetrisBlock> BlockBPClass(TEXT("/Game/TetrisBlock"));
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
            CameraActor->SetActorLocation(FVector(-2500.0f, 500.0f, 1000.0f)); // 블록 정면을 더 넓게 볼 수 있는 위치
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
    InputComponent->BindAction("HardDrop", IE_Pressed, this, &ATetrisBoard::HardDrop);

    // 경계 프레임 생성
    CreateBorderFrames();
    
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

    // 땅 충돌 기준 변경: 여기서는 땅을 Z = 50.0f로 취급합니다.
    const float GroundLevel = 50.0f;

    for (UStaticMeshComponent* Mesh : ActiveBlock->GetBlockMeshes())
    {
        // 회전 후 각 큐브의 상대 위치를 기준으로 보드상의 절대 위치 계산
        FVector RelativeLocation = Mesh->GetRelativeLocation();
        FVector BlockLocation = Location + RelativeLocation;

        // 보드 경계 충돌 감지 (Y축)
        if (BlockLocation.Y < 0.0f || BlockLocation.Y >= BoardWidth * 100.0f)
        {
            return true; // 보드 경계를 벗어남
        }

        // 땅(바닥) 충돌 감지: 블럭이 땅(Z = 50.0f) 아래로 내려가면 충돌로 판단
        if (BlockLocation.Z < GroundLevel)
        {
            return true;
        }

        // 다른 블록과의 충돌 감지 (Y, Z축 격자 기준)
        int YIndex = FMath::FloorToInt(BlockLocation.Y / 100.0f);
        int ZIndex = FMath::FloorToInt(BlockLocation.Z / 100.0f);

        if (YIndex >= 0 && YIndex < BoardWidth && ZIndex >= 0 && ZIndex < BoardHeight)
        {
            if (Board[YIndex][ZIndex]) // 이미 블록이 존재하면 충돌
            {
                return true;
            }
        }
    }
    return false; // 충돌 없음
}

void ATetrisBoard::ClearFullRows()
{
    int NumRowsCleared = 0;
    
    for (int32 z = 0; z < BoardHeight; z++)
    {
        FString RowState;
        int TrueCount = 0;
        
        for (int32 y = 0; y < BoardWidth; y++)
        {
            bool cell = Board[y][z];
            RowState += (cell ? "1" : "0");
            if (cell)
            {
                TrueCount++;
            }
        }
        
        if (TrueCount == BoardWidth)
        {
            UE_LOG(LogTemp, Warning, TEXT("Height %d is full and will be cleared."), z);
            NumRowsCleared++;
            
            // 해당 높이의 모든 블록 메시를 찾아서 제거
            TArray<AActor*> FoundActors;
            UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATetrisBlock::StaticClass(), FoundActors);
            
            // 먼저 제거할 메시와 이동할 메시를 분리하여 처리
            TArray<UStaticMeshComponent*> MeshesToDestroy;
            TArray<FVector> NewLocations;
            TArray<UStaticMeshComponent*> MeshesToMove;

            for (AActor* Actor : FoundActors)
            {
                ATetrisBlock* Block = Cast<ATetrisBlock>(Actor);
                if (Block && Block != ActiveBlock)
                {
                    // const TArray를 복사하여 사용
                    const TArray<UStaticMeshComponent*>& Meshes = Block->GetBlockMeshes();
                    for (int32 i = 0; i < Meshes.Num(); i++)
                    {
                        if (UStaticMeshComponent* Mesh = Meshes[i])
                        {
                            FVector Location = Mesh->GetComponentLocation();
                            int32 MeshZ = FMath::RoundToInt(Location.Z / 100.0f);
                            
                            if (MeshZ == z)
                            {
                                MeshesToDestroy.Add(Mesh);
                            }
                            else if (MeshZ > z)
                            {
                                MeshesToMove.Add(Mesh);
                                NewLocations.Add(Location - FVector(0.0f, 0.0f, 100.0f));
                            }
                        }
                    }
                }
            }

            // 메시 제거
            for (UStaticMeshComponent* Mesh : MeshesToDestroy)
            {
                if (Mesh && Mesh->IsValidLowLevel())
                {
                    Mesh->DestroyComponent();
                }
            }

            // 메시 이동
            for (int32 i = 0; i < MeshesToMove.Num(); i++)
            {
                if (MeshesToMove[i] && MeshesToMove[i]->IsValidLowLevel())
                {
                    MeshesToMove[i]->SetWorldLocation(NewLocations[i]);
                }
            }

            // 보드 배열 업데이트
            for (int32 currentZ = z; currentZ < BoardHeight - 1; currentZ++)
            {
                for (int32 y = 0; y < BoardWidth; y++)
                {
                    Board[y][currentZ] = Board[y][currentZ + 1];
                }
            }
            
            // 최상단 행을 비움
            for (int32 y = 0; y < BoardWidth; y++)
            {
                Board[y][BoardHeight - 1] = false;
            }
            
            z--; // 행이 제거되었으므로 같은 위치를 다시 검사
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Height %d: %s (True count: %d)"), z, *RowState, TrueCount);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Total rows cleared: %d"), NumRowsCleared);
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

        // 각 큐브의 상대 위치를 기준으로 X축 회전
        for (UStaticMeshComponent* Mesh : ActiveBlock->GetBlockMeshes())
        {
            FVector RelativeLocation = Mesh->GetRelativeLocation() - BlockCenter;

            // X축 기준으로 90도 회전 변환
            float NewY = -RelativeLocation.Z;
            float NewZ = RelativeLocation.Y;

            // 새로운 위치 설정 (중심점 기준으로 이동)
            FVector NewLocation = FVector(RelativeLocation.X, NewY, NewZ) + BlockCenter;

            // 격자에 맞게 위치 보정
            NewLocation.X = FMath::RoundToFloat(NewLocation.X / 100.0f) * 100.0f;
            NewLocation.Y = FMath::RoundToFloat(NewLocation.Y / 100.0f) * 100.0f;
            NewLocation.Z = FMath::RoundToFloat(NewLocation.Z / 100.0f) * 100.0f;

            Mesh->SetRelativeLocation(NewLocation);
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

void ATetrisBoard::HardDrop()
{
    if (!ActiveBlock)
    {
        return;
    }

    FVector Offset = FVector(0.0f, 0.0f, -100.0f);
    // 충돌이 발생할 때까지 아래로 이동
    while (!HasCollision(ActiveBlock->GetActorLocation() + Offset))
    {
        ActiveBlock->Move(Offset);
    }

    // 블록 고정: 현재 ActiveBlock의 각 큐브를 보드에 반영
    for (UStaticMeshComponent* Mesh : ActiveBlock->GetBlockMeshes())
    {
        FVector BlockLocation = Mesh->GetComponentLocation();
        int YIndex = FMath::FloorToInt(BlockLocation.Y / 100.0f);
        int ZIndex = FMath::FloorToInt(BlockLocation.Z / 100.0f);

        if (YIndex >= 0 && YIndex < BoardWidth && ZIndex >= 0 && ZIndex < BoardHeight)
        {
            Board[YIndex][ZIndex] = true;
        }
    }

    ClearFullRows(); // 줄 제거
    SpawnBlock();    // 새 블록 생성
}

void ATetrisBoard::CreateBorderFrames()
{
    // CubeMesh와 BorderMaterial 로드
    UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
    if(!CubeMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load Cube Mesh from /Engine/BasicShapes/Cube"));
        return;
    }
    
    UMaterial* BorderMaterial = LoadObject<UMaterial>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
    if(!BorderMaterial)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load Border Material from /Engine/BasicShapes/BasicShapeMaterial"));
        return;
    }
    
    // 경계 액터 생성: 왼쪽과 오른쪽 경계를 각 z 레벨마다 스폰 (AStaticMeshActor 사용)
    for (int32 z = 0; z < BoardHeight; z++)
    {
        // 왼쪽 경계 액터 생성
        FVector LeftLocation = FVector(-50.0f, -100.0f, z * 100.0f + 50.0f);
        FRotator SpawnRotator = FRotator::ZeroRotator;
        AStaticMeshActor* LeftBorder = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), LeftLocation, SpawnRotator);
        if (LeftBorder)
        {
            LeftBorder->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
            LeftBorder->GetStaticMeshComponent()->SetMaterial(0, BorderMaterial);
            LeftBorder->SetActorScale3D(DefaultScale);  
            LeftBorder->GetStaticMeshComponent()->SetMobility(EComponentMobility::Static);
            UE_LOG(LogTemp, Log, TEXT("Left border spawned at Z: %d, Location: %s, Scale: %s"),
                z, *LeftLocation.ToString(), *DefaultScale.ToString());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to spawn left border at Z: %d"), z);
        }
        
        // 오른쪽 경계 액터 생성
        FVector RightLocation = FVector(-50.0f, BoardWidth * 100.0f, z * 100.0f + 50.0f);
        AStaticMeshActor* RightBorder = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), RightLocation, SpawnRotator);
        if (RightBorder)
        {
            RightBorder->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
            RightBorder->GetStaticMeshComponent()->SetMaterial(0, BorderMaterial);
            RightBorder->SetActorScale3D(DefaultScale);
            RightBorder->GetStaticMeshComponent()->SetMobility(EComponentMobility::Static);
            UE_LOG(LogTemp, Log, TEXT("Right border spawned at Z: %d, Location: %s, Scale: %s"),
                z, *RightLocation.ToString(), *DefaultScale.ToString());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to spawn right border at Z: %d"), z);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Border actors created using StaticMeshActor."));
}