#include "TetrisBoard.h"
#include "../Blocks/TetrisBlock.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraActor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"    // AStaticMeshActor 사용

ATetrisBoard::ATetrisBoard()
{
    PrimaryActorTick.bCanEverTick = true;

    // Root 컴포넌트 생성 (Board의 기준 위치)
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // 기본 스케일 설정 (경계 프레임에 사용)
    DefaultScale = FVector(0.1f, 0.1f, 1.0f);

    // 사용될 블록 클래스를 설정 ("/Game/TetrisBlock" 에셋)
    static ConstructorHelpers::FClassFinder<ATetrisBlock> BlockBPClass(TEXT("/Game/TetrisBlock"));
    if (BlockBPClass.Class)
    {
        BlockClass = BlockBPClass.Class;
    }
}

void ATetrisBoard::BeginPlay()
{
    Super::BeginPlay();

    // 플레이어 컨트롤러와 카메라 액터 설정
    if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
    {
        if (ACameraActor* CameraActor = GetWorld()->SpawnActor<ACameraActor>())
        {
            // 카메라의 위치, 회전 설정
            CameraActor->SetActorLocation(FVector(-2500.f, 500.f, 1000.f)); // 블록 정면을 넓게 보기 위한 위치
            CameraActor->SetActorRotation(FRotator::ZeroRotator);
            PlayerController->SetViewTarget(CameraActor);
        }
    }

    // 입력 바인딩
    EnableInput(GetWorld()->GetFirstPlayerController());
    InputComponent->BindAction("MoveLeft", IE_Pressed, this, &ATetrisBoard::MoveLeft);
    InputComponent->BindAction("MoveRight", IE_Pressed, this, &ATetrisBoard::MoveRight);
    InputComponent->BindAction("MoveDown", IE_Pressed, this, &ATetrisBoard::MoveDown);
    InputComponent->BindAction("Rotate", IE_Pressed, this, &ATetrisBoard::RotateBlock);
    InputComponent->BindAction("HardDrop", IE_Pressed, this, &ATetrisBoard::HardDrop);

    // 경계 프레임 생성 및 초기 블록 생성
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

    // 블록 낙하 시간 누적 처리
    TimeSinceLastFall += DeltaTime;
    if (ActiveBlock && TimeSinceLastFall >= BlockFallInterval)
    {
        FVector NewLocation = ActiveBlock->GetActorLocation();
        ActiveBlock->SetActorLocation(NewLocation);
        TimeSinceLastFall = 0.f;

        // 충돌 감지 시 블록 고정 후 체인지
        if (HasCollision(NewLocation))
        {
            ActiveBlock->SetActorLocation(NewLocation + FVector(0.f, 0.f, 100.f));
            ClearFullRows();
            SpawnBlock();
        }
    }
}

void ATetrisBoard::SpawnBlock()
{
    // 이전 블록의 위치를 보드 배열에 반영
    if (ActiveBlock)
    {
        for (UStaticMeshComponent* Mesh : ActiveBlock->GetBlockMeshes())
        {
            FVector BlockLocation = Mesh->GetComponentLocation();
            int YIndex = FMath::FloorToInt(BlockLocation.Y / 100.f);
            int ZIndex = FMath::FloorToInt(BlockLocation.Z / 100.f);
            if (YIndex >= 0 && YIndex < BoardWidth && ZIndex >= 0 && ZIndex < BoardHeight)
            {
                Board[YIndex][ZIndex] = true;
            }
        }
    }

    // 새로운 블록 생성 (보드 중앙 위쪽)
    FVector SpawnLocation = FVector(0.f, (BoardWidth / 2) * 100.f, BoardHeight * 100.f);
    FRotator SpawnRotation = FRotator::ZeroRotator;
    if (BlockClass)
    {
        ActiveBlock = GetWorld()->SpawnActor<ATetrisBlock>(BlockClass, SpawnLocation, SpawnRotation);
        if (ActiveBlock)
        {
            TArray<FVector> BlockShape;
            int32 ShapeType = FMath::RandRange(0,6);
            switch (ShapeType)
            {
                case 0: BlockShape = IBlock; break;      // 빨강
                case 1: BlockShape = OBlock; break;      // 주황
                case 2: BlockShape = TBlock; break;      // 노랑
                case 3: BlockShape = LBlock; break;      // 초록
                case 4: BlockShape = JBlock; break;      // 파랑
                case 5: BlockShape = SBlock; break;      // 남색
                case 6: BlockShape = ZBlock; break;      // 보라
            }
            ActiveBlock->InitializeBlock(BlockShape);
            
            FLinearColor ColorToSet;
            switch (ShapeType)
            {
                case 0: ColorToSet = FLinearColor::Red; break;
                case 1: ColorToSet = FLinearColor(1.f, 0.5f, 0.f, 1.f); break; // 주황
                case 2: ColorToSet = FLinearColor::Yellow; break;
                case 3: ColorToSet = FLinearColor::Green; break;
                case 4: ColorToSet = FLinearColor::Blue; break;
                case 5: ColorToSet = FLinearColor(0.29f, 0.f, 0.51f, 1.f); break; // 남색
                case 6: ColorToSet = FLinearColor(0.56f, 0.f, 1.f, 1.f); break;      // 보라
            }
            
            for (UStaticMeshComponent* Mesh : ActiveBlock->GetBlockMeshes())
            {
                if (Mesh)
                {
                    UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(Mesh->GetMaterial(0), this);
                    if (DynMat)
                    {
                        // "Color" 파라미터가 머티리얼에 존재하는지 확인할 것
                        DynMat->SetVectorParameterValue(TEXT("Color"), ColorToSet);
                        Mesh->SetMaterial(0, DynMat);
                    }
                }
            }
            
            // 게임 오버 상태 체크
            if (HasCollision(ActiveBlock->GetActorLocation()))
            {
                bIsGameOver = true;
                UE_LOG(LogTemp, Error, TEXT("Game Over! Block spawned in a collision state."));
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("New block spawned at: %s"), *SpawnLocation.ToString());
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

// 땅 기준 Z값을 0.0f로 설정하여 블럭들이 땅과 정확히 일치하도록 함
bool ATetrisBoard::HasCollision(const FVector& Location)
{
    if (!ActiveBlock)
    {
        return false;
    }

    const float GroundLevel = 0.0f; // 땅 기준을 0으로 설정
    for (UStaticMeshComponent* Mesh : ActiveBlock->GetBlockMeshes())
    {
        FVector RelativeLocation = Mesh->GetRelativeLocation();
        FVector BlockLocation = Location + RelativeLocation;
        // Y축 경계 체크 (보드 폭)
        if (BlockLocation.Y < 0.f || BlockLocation.Y >= BoardWidth * 100.f)
        {
            return true;
        }
        // 땅 아래인지 체크
        if (BlockLocation.Z < GroundLevel)
        {
            return true;
        }
        int YIndex = FMath::FloorToInt(BlockLocation.Y / 100.f);
        int ZIndex = FMath::FloorToInt(BlockLocation.Z / 100.f);
        if (YIndex >= 0 && YIndex < BoardWidth && ZIndex >= 0 && ZIndex < BoardHeight)
        {
            if (Board[YIndex][ZIndex])
            {
                return true;
            }
        }
    }
    return false;
}

// 블록의 각 큐브 컴포넌트의 월드 바운드를 이용하여 땅과 접촉했는지 검사
bool ATetrisBoard::IsBlockTouchingGround(ATetrisBlock* Block)
{
    if (!Block)
    {
        return false;
    }
    
    // 땅 기준 Z값 (0으로 설정)
    const float GroundLevel = 0.0f;
    
    // 각 컴포넌트의 바운드를 계산
    for (UStaticMeshComponent* Mesh : Block->GetBlockMeshes())
    {
        // Bounds는 이미 월드 공간의 바운드를 제공하므로,
        // 최소 Z값이 GroundLevel 이하이면 땅과 닿은 것으로 판단
        FBoxSphereBounds Bounds = Mesh->Bounds;
        if (Bounds.Origin.Z - Bounds.BoxExtent.Z <= GroundLevel)
        {
            return true;
        }
    }
    return false;
}

// 블록의 가장 낮은 Z값을 계산하고, 바닥으로부터 50.0f 위에 위치시킵니다.
void ATetrisBoard::AdjustBlockAboveGround(ATetrisBlock* Block)
{
    if (!Block)
    {
        return;
    }

    float MinZ = FLT_MAX;
    for (UStaticMeshComponent* Mesh : Block->GetBlockMeshes())
    {
        FBoxSphereBounds Bounds = Mesh->Bounds;
        float MeshMinZ = Bounds.Origin.Z - Bounds.BoxExtent.Z;
        if (MeshMinZ < MinZ)
        {
            MinZ = MeshMinZ;
        }
    }

    // 만약 블록의 최소 Z값이 50보다 낮다면, 50까지의 차이만큼 위로 이동
    float OffsetZ = 50.f - MinZ; 
    if (OffsetZ > 0.f)
    {
        Block->SetActorLocation(Block->GetActorLocation() + FVector(0.f, 0.f, OffsetZ));
        UE_LOG(LogTemp, Log, TEXT("Block adjusted upward by %.1f to keep 50.f offset from ground."), OffsetZ);
    }
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

            TArray<AActor*> FoundActors;
            UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATetrisBlock::StaticClass(), FoundActors);
            TArray<UStaticMeshComponent*> MeshesToDestroy;
            TArray<UStaticMeshComponent*> MeshesToMove;
            TArray<FVector> NewLocations;
            for (AActor* Actor : FoundActors)
            {
                if (ATetrisBlock* Block = Cast<ATetrisBlock>(Actor))
                {
                    if (Block == ActiveBlock) continue;
                    const TArray<UStaticMeshComponent*>& Meshes = Block->GetBlockMeshes();
                    for (UStaticMeshComponent* Mesh : Meshes)
                    {
                        if (Mesh)
                        {
                            FVector Loc = Mesh->GetComponentLocation();
                            int32 MeshZ = FMath::RoundToInt(Loc.Z / 100.f);
                            if (MeshZ == z)
                            {
                                MeshesToDestroy.Add(Mesh);
                            }
                            else if (MeshZ > z)
                            {
                                MeshesToMove.Add(Mesh);
                                NewLocations.Add(Loc - FVector(0.f, 0.f, 100.f));
                            }
                        }
                    }
                }
            }
            for (UStaticMeshComponent* Mesh : MeshesToDestroy)
            {
                if (Mesh && Mesh->IsValidLowLevel())
                {
                    Mesh->DestroyComponent();
                }
            }
            for (int32 i = 0; i < MeshesToMove.Num(); i++)
            {
                if (MeshesToMove[i] && MeshesToMove[i]->IsValidLowLevel())
                {
                    MeshesToMove[i]->SetWorldLocation(NewLocations[i]);
                }
            }
            for (int32 currentZ = z; currentZ < BoardHeight - 1; currentZ++)
            {
                for (int32 y = 0; y < BoardWidth; y++)
                {
                    Board[y][currentZ] = Board[y][currentZ + 1];
                }
            }
            for (int32 y = 0; y < BoardWidth; y++)
            {
                Board[y][BoardHeight - 1] = false;
            }
            z--;
        }
        UE_LOG(LogTemp, Warning, TEXT("Height %d: %s (True count: %d)"), z, *RowState, TrueCount);
    }
    UE_LOG(LogTemp, Warning, TEXT("Total rows cleared: %d"), NumRowsCleared);
}

void ATetrisBoard::MoveLeft()
{
    if (ActiveBlock)
    {
        FVector Offset = FVector(0.f, -100.f, 0.f); // Y축 왼쪽 이동
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
        FVector Offset = FVector(0.f, 100.f, 0.f); // Y축 오른쪽 이동
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
        FVector Offset = FVector(0.f, 0.f, -100.f); // Z축 아래로 이동
        FVector NewLocation = ActiveBlock->GetActorLocation() + Offset;
        if (!HasCollision(NewLocation))
        {
            ActiveBlock->Move(Offset);
        }
        else
        {
            // 충돌 시 블록 고정 후 새로운 블록 생성
            for (UStaticMeshComponent* Mesh : ActiveBlock->GetBlockMeshes())
            {
                FVector BlockLocation = Mesh->GetComponentLocation();
                int YIndex = FMath::FloorToInt(BlockLocation.Y / 100.f);
                int ZIndex = FMath::FloorToInt(BlockLocation.Z / 100.f);
                if (YIndex >= 0 && YIndex < BoardWidth && ZIndex >= 0 && ZIndex < BoardHeight)
                {
                    Board[YIndex][ZIndex] = true;
                }
            }
            ClearFullRows();
            SpawnBlock();
        }
    }
}

void ATetrisBoard::RotateBlock()
{
    if (ActiveBlock)
    {
        TArray<FVector> OriginalLocations;
        for (UStaticMeshComponent* Mesh : ActiveBlock->GetBlockMeshes())
        {
            OriginalLocations.Add(Mesh->GetRelativeLocation());
        }

        // 블록 중심 계산
        FVector BlockCenter = FVector::ZeroVector;
        for (UStaticMeshComponent* Mesh : ActiveBlock->GetBlockMeshes())
        {
            BlockCenter += Mesh->GetRelativeLocation();
        }
        BlockCenter /= ActiveBlock->GetBlockMeshes().Num();

        // X축 기준 90도 회전
        for (UStaticMeshComponent* Mesh : ActiveBlock->GetBlockMeshes())
        {
            FVector RelativeLocation = Mesh->GetRelativeLocation() - BlockCenter;
            float NewY = -RelativeLocation.Z;
            float NewZ = RelativeLocation.Y;
            FVector NewLocation = FVector(RelativeLocation.X, NewY, NewZ) + BlockCenter;
            NewLocation.X = FMath::RoundToFloat(NewLocation.X / 100.f) * 100.f;
            NewLocation.Y = FMath::RoundToFloat(NewLocation.Y / 100.f) * 100.f;
            NewLocation.Z = FMath::RoundToFloat(NewLocation.Z / 100.f) * 100.f;
            Mesh->SetRelativeLocation(NewLocation);
        }

        if (HasCollision(ActiveBlock->GetActorLocation()))
        {
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
    FVector Offset = FVector(0.f, 0.f, -100.f);
    while (!HasCollision(ActiveBlock->GetActorLocation() + Offset))
    {
        ActiveBlock->Move(Offset);
    }
    for (UStaticMeshComponent* Mesh : ActiveBlock->GetBlockMeshes())
    {
        FVector BlockLocation = Mesh->GetComponentLocation();
        int YIndex = FMath::FloorToInt(BlockLocation.Y / 100.f);
        int ZIndex = FMath::FloorToInt(BlockLocation.Z / 100.f);
        if (YIndex >= 0 && YIndex < BoardWidth && ZIndex >= 0 && ZIndex < BoardHeight)
        {
            Board[YIndex][ZIndex] = true;
        }
    }
    ClearFullRows();
    SpawnBlock();
}

void ATetrisBoard::CreateBorderFrames()
{
    // CubeMesh와 BorderMaterial 로드
    UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
    if (!CubeMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load Cube Mesh from /Engine/BasicShapes/Cube"));
        return;
    }
    
    UMaterial* BorderMaterial = LoadObject<UMaterial>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
    if (!BorderMaterial)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load Border Material from /Engine/BasicShapes/BasicShapeMaterial"));
        return;
    }
    
    // 경계 액터 생성 (각 z 레벨마다 좌우 생성)
    for (int32 z = 0; z < BoardHeight; z++)
    {
        FRotator SpawnRotator = FRotator::ZeroRotator;
        
        // 왼쪽 경계 생성: X=0, Y=50.0f, Z = z*100.f
        FVector LeftLocation = FVector(0.f, -50.f, z * 100.f);
        if (AStaticMeshActor* LeftBorder = GetWorld()->SpawnActor<AStaticMeshActor>(
                AStaticMeshActor::StaticClass(), LeftLocation, SpawnRotator))
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
        
        // 오른쪽 경계 생성: X=0, Y=BoardWidth * 100.f - 50.0f, Z = z*100.f
        FVector RightLocation = FVector(0.f, BoardWidth * 100.f - 50.f, z * 100.f);
        if (AStaticMeshActor* RightBorder = GetWorld()->SpawnActor<AStaticMeshActor>(
                AStaticMeshActor::StaticClass(), RightLocation, SpawnRotator))
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