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

    // Board 배열 초기화: BoardWidth 만큼 행, 각 행에 BoardHeight개의 false값으로 초기화
    Board.SetNum(BoardWidth);
    for (int32 i = 0; i < BoardWidth; i++)
    {
        Board[i].Init(false, BoardHeight);
    }

    // 플레이어 컨트롤러와 카메라 액터 설정
    if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
    {
        if (ACameraActor* CameraActor = GetWorld()->SpawnActor<ACameraActor>())
        {
            // 카메라의 위치, 회전 설정
            CameraActor->SetActorLocation(FVector(-2500.f, (BoardWidth / 2) * 100.f, 1250.f)); // 블록 정면을 넓게 보기 위한 위치
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
        UE_LOG(LogTemp, Warning, TEXT("Game over."));
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
    FVector SpawnLocation = FVector(0.f, (BoardWidth / 2) * 100.f, BoardHeight * 100.f - 55.f);
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
            
            //UMaterial* LoadedMaterial = LoadObject<UMaterial>(nullptr, TEXT("/Script/Engine.Material'/Game/BlockMaterial.BlockMaterial'"));
            if (!LoadedMaterial)
            {
                UE_LOG(LogTemp, Error, TEXT("사용자 정의 머티리얼 로드에 실패했습니다: /Script/Engine.Material'/Game/BlockMaterial.BlockMaterial'"));
                return;
            }
            
            FLinearColor ColorToSet;
            switch (ShapeType)
            {
                case 0: ColorToSet = FLinearColor::Red; break;
                case 1: ColorToSet = FLinearColor(1.f, 0.5f, 0.f, 1.f); break; // 주황
                case 2: ColorToSet = FLinearColor::Yellow; break;
                case 3: ColorToSet = FLinearColor::Green; break;
                case 4: ColorToSet = FLinearColor::Blue; break;
                case 5: ColorToSet = FLinearColor(0.29f, 0.f, 0.51f, 1.f); break; // 남색
                case 6: ColorToSet = FLinearColor(0.56f, 0.f, 1.f, 1.f); break; // 보라
            }
            
            for (UStaticMeshComponent* Mesh : ActiveBlock->GetBlockMeshes())
            {
                if (Mesh)
                {
                    UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(LoadedMaterial, this);
                    if (DynMat)
                    {
                        DynMat->SetVectorParameterValue(TEXT("Color"), ColorToSet);
                        Mesh->SetMaterial(0, DynMat);
                    }
                }
            }
            
            // 게임 오버 상태 체크
            if (HasCollision(ActiveBlock->GetActorLocation()))
            {
                bIsGameOver = true;
                UE_LOG(LogTemp, Error, TEXT("충돌 상태로 인해 블록 생성에 실패했습니다 (게임 오버)."));
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("새 블록 생성 완료: %s"), *SpawnLocation.ToString());
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("ActiveBlock 생성에 실패했습니다."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("BlockClass가 설정되어 있지 않습니다."));
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
    
    const float GroundLevel = 0.0f; // '땅' 기준 X값 0
    for (UStaticMeshComponent* Mesh : Block->GetBlockMeshes())
    {
        FBoxSphereBounds Bounds = Mesh->Bounds;
        // X축 기준 최소값 계산
        if (Bounds.Origin.X - Bounds.BoxExtent.X <= GroundLevel)
        {
            return true;
        }
    }
    return false;
}

void ATetrisBoard::ClearFullRows()
{
    // 1. Board 배열을 통해 채워진(Full) 행의 인덱스를 수집
    TArray<int32> FullRows;
    for (int32 z = 0; z < BoardHeight; z++)
    {
        int count = 0;
        for (int32 y = 0; y < BoardWidth; y++)
        {
            if (Board[y][z])
            {
                count++;
            }
        }
        if (count == BoardWidth)
        {
            FullRows.Add(z);
        }
    }
    
    if (FullRows.Num() == 0)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ClearFullRows: %d 행이 삭제 대상입니다."), FullRows.Num());
    
    // 2. 모든 ATetrisBlock 액터(ActiveBlock 포함)를 순회하면서 각 큐브 처리
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATetrisBlock::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (ATetrisBlock* Block = Cast<ATetrisBlock>(Actor))
        {
            const TArray<UStaticMeshComponent*>& Meshes = Block->GetBlockMeshes();
            // 역순 순회 (삭제 안전)
            for (int32 i = Meshes.Num() - 1; i >= 0; i--)
            {
                UStaticMeshComponent* Mesh = Meshes[i];
                if (Mesh && Mesh->IsValidLowLevel())
                {
                    FVector Loc = Mesh->GetComponentLocation();
                    // 각 셀의 크기가 100.f라고 가정하여 행 인덱스를 계산
                    int32 MeshRow = FMath::RoundToInt(Loc.Z / 100.f);
                    
                    // full row에 속한다면 큐브를 화면 밖(-200.f)으로 이동 후 삭제
                    if (FullRows.Contains(MeshRow))
                    {
                        FVector NewLoc = Loc;
                        NewLoc.Z = -200.f;
                        Mesh->SetWorldLocation(NewLoc);
                        Mesh->DestroyComponent();
                        UE_LOG(LogTemp, Log, TEXT("행 %d에 위치한 큐브 clear 처리됨: z를 -200.f로 설정"), MeshRow);
                    }
                    else
                    {
                        // 이 큐브보다 아래에 몇 개의 full row가 있는지 계산하여 아래로 이동
                        int32 ShiftCount = 0;
                        for (int32 ClearedRow : FullRows)
                        {
                            if (MeshRow > ClearedRow)
                            {
                                ShiftCount++;
                            }
                        }
                        if (ShiftCount > 0)
                        {
                            FVector NewLoc = Loc - FVector(0.f, 0.f, 100.f * ShiftCount);
                            Mesh->SetWorldLocation(NewLoc);
                        }
                    }
                }
            }
        }
    }
    
    // 3. Board 배열 재구성 (남은 큐브들의 실제 위치 기준)
    // 먼저 Board 배열 초기화
    for (int32 y = 0; y < BoardWidth; y++)
    {
        Board[y].Init(false, BoardHeight);
    }
    // 다시 모든 블록 액터의 유효한 큐브 위치를 Board에 반영
    for (AActor* Actor : FoundActors)
    {
        if (ATetrisBlock* Block = Cast<ATetrisBlock>(Actor))
        {
            const TArray<UStaticMeshComponent*>& Meshes = Block->GetBlockMeshes();
            for (UStaticMeshComponent* Mesh : Meshes)
            {
                if (Mesh && Mesh->IsValidLowLevel() && !Mesh->IsBeingDestroyed())
                {
                    FVector Loc = Mesh->GetComponentLocation();
                    int32 yIndex = FMath::FloorToInt(Loc.Y / 100.f);
                    int32 zIndex = FMath::FloorToInt(Loc.Z / 100.f);
                    if (yIndex >= 0 && yIndex < BoardWidth && zIndex >= 0 && zIndex < BoardHeight)
                    {
                        Board[yIndex][zIndex] = true;
                    }
                }
            }
        }
    }
    
    // 4. 빈 블록 액터 정리: 큐브가 남아있지 않으면 블록 액터 삭제
    for (AActor* Actor : FoundActors)
    {
        if (ATetrisBlock* Block = Cast<ATetrisBlock>(Actor))
        {
            bool bHasCube = false;
            const TArray<UStaticMeshComponent*>& Meshes = Block->GetBlockMeshes();
            for (UStaticMeshComponent* Mesh : Meshes)
            {
                if (Mesh && Mesh->IsValidLowLevel() && !Mesh->IsBeingDestroyed())
                {
                    bHasCube = true;
                    break;
                }
            }
            if (!bHasCube)
            {
                Block->Destroy();
                UE_LOG(LogTemp, Log, TEXT("큐브 없는 블록 액터 삭제됨"));
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("총 %d개의 행이 삭제되었습니다."), FullRows.Num());
}

void ATetrisBoard::MoveLeft()
{
    if (bIsGameOver)
    {
        UE_LOG(LogTemp, Warning, TEXT("게임 오버 상태입니다. 조작이 불가합니다."));
        return;
    }
    
    if (ActiveBlock)
    {
        FVector Offset = FVector(0.f, -100.f, 0.f); // Y축 왼쪽 이동
        if (!HasCollision(ActiveBlock->GetActorLocation() + Offset))
        {
            ActiveBlock->Move(Offset);
            UE_LOG(LogTemp, Log, TEXT("왼쪽으로 이동했습니다."));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("왼쪽으로 이동 시 충돌이 발생했습니다."));
        }
    }
}

void ATetrisBoard::MoveRight()
{
    if (bIsGameOver)
    {
        UE_LOG(LogTemp, Warning, TEXT("게임 오버 상태입니다. 조작이 불가합니다."));
        return;
    }
    
    if (ActiveBlock)
    {
        FVector Offset = FVector(0.f, 100.f, 0.f); // Y축 오른쪽 이동
        if (!HasCollision(ActiveBlock->GetActorLocation() + Offset))
        {
            ActiveBlock->Move(Offset);
            UE_LOG(LogTemp, Log, TEXT("오른쪽으로 이동했습니다."));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("오른쪽으로 이동 시 충돌이 발생했습니다."));
        }
    }
}

void ATetrisBoard::MoveDown()
{
    if (bIsGameOver)
    {
        UE_LOG(LogTemp, Warning, TEXT("게임 오버 상태입니다. 조작이 불가합니다."));
        return;
    }
    
    if (ActiveBlock)
    {
        FVector Offset = FVector(0.f, 0.f, -100.f); // Z축 아래로 이동
        FVector NewLocation = ActiveBlock->GetActorLocation() + Offset;
        if (!HasCollision(NewLocation))
        {
            ActiveBlock->Move(Offset);
            UE_LOG(LogTemp, Log, TEXT("아래로 이동했습니다."));
        }
        else
        {
            // 충돌 시 블록 고정 처리 (조작 불가)
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
            UE_LOG(LogTemp, Log, TEXT("아래쪽 충돌로 인해 블록을 고정합니다."));
            ClearFullRows();
            SpawnBlock();
        }
    }
}

void ATetrisBoard::RotateBlock()
{
    if (bIsGameOver)
    {
        UE_LOG(LogTemp, Warning, TEXT("게임 오버 상태입니다. 조작이 불가합니다."));
        return;
    }
    
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
            UE_LOG(LogTemp, Warning, TEXT("회전 시 충돌로 인해 회전이 취소되었습니다."));
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("블록이 성공적으로 회전되었습니다."));
        }
    }
}

void ATetrisBoard::HardDrop()
{
    if (bIsGameOver)
    {
        UE_LOG(LogTemp, Warning, TEXT("게임 오버 상태입니다. 조작이 불가합니다."));
        return;
    }
    
    if (!ActiveBlock)
    {
        UE_LOG(LogTemp, Warning, TEXT("활성 블록이 없습니다."));
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
    // CubeMesh와 경계 머티리얼 로드
    UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
    if (!CubeMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("Cube 메시 로드에 실패했습니다: /Engine/BasicShapes/Cube"));
        return;
    }
    
    UMaterial* BorderMaterial = LoadObject<UMaterial>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
    if (!BorderMaterial)
    {
        UE_LOG(LogTemp, Error, TEXT("경계 머티리얼 로드에 실패했습니다: /Engine/BasicShapes/BasicShapeMaterial"));
        return;
    }
    
    // 각 z 레벨마다 좌우 경계 액터 생성
    for (int32 z = 0; z < BoardHeight; z++)
    {
        FRotator SpawnRotator = FRotator::ZeroRotator;
        
        // 왼쪽 경계 생성: X=0, Y=-50.0f, Z = z*100.f
        FVector LeftLocation = FVector(0.f, -50.f, z * 100.f);
        if (AStaticMeshActor* LeftBorder = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), LeftLocation, SpawnRotator))
        {
            LeftBorder->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
            LeftBorder->GetStaticMeshComponent()->SetMaterial(0, BorderMaterial);
            LeftBorder->SetActorScale3D(DefaultScale);
            LeftBorder->GetStaticMeshComponent()->SetMobility(EComponentMobility::Static);
            //UE_LOG(LogTemp, Log, TEXT("왼쪽 경계 생성됨 (높이 %d): 위치: %s, 스케일: %s"), 
            //       z, *LeftLocation.ToString(), *DefaultScale.ToString());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("높이 %d에서 왼쪽 경계 생성에 실패했습니다."), z);
        }
        
        // 오른쪽 경계 생성: X=0, Y=BoardWidth * 100.f - 50.0f, Z = z*100.f
        FVector RightLocation = FVector(0.f, BoardWidth * 100.f - 50.f, z * 100.f);
        if (AStaticMeshActor* RightBorder = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), RightLocation, SpawnRotator))
        {
            RightBorder->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
            RightBorder->GetStaticMeshComponent()->SetMaterial(0, BorderMaterial);
            RightBorder->SetActorScale3D(DefaultScale);
            RightBorder->GetStaticMeshComponent()->SetMobility(EComponentMobility::Static);
            //UE_LOG(LogTemp, Log, TEXT("오른쪽 경계 생성됨 (높이 %d): 위치: %s, 스케일: %s"),
            //       z, *RightLocation.ToString(), *DefaultScale.ToString());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("높이 %d에서 오른쪽 경계 생성에 실패했습니다."), z);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("StaticMeshActor를 사용하여 경계 액터 생성이 완료되었습니다."));
}