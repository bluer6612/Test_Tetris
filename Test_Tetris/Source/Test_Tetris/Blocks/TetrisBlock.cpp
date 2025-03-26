#include "TetrisBlock.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

ATetrisBlock::ATetrisBlock()
{
    PrimaryActorTick.bCanEverTick = true;

    // 기본 큐브 메쉬 설정
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        CubeMesh = CubeMeshAsset.Object;
    }
}

void ATetrisBlock::BeginPlay()
{
    Super::BeginPlay();
}

void ATetrisBlock::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ATetrisBlock::InitializeBlock(const TArray<FVector>& BlockShape)
{
    // 기존 블록 메쉬 제거
    for (UStaticMeshComponent* Mesh : BlockMeshes)
    {
        if (Mesh)
        {
            Mesh->DestroyComponent();
        }
    }
    BlockMeshes.Empty();

    // 새로운 블록 메쉬 생성
    for (const FVector& Offset : BlockShape)
    {
        UStaticMeshComponent* NewMesh = NewObject<UStaticMeshComponent>(this);
        if (NewMesh)
        {
            NewMesh->SetStaticMesh(CubeMesh);
            NewMesh->SetupAttachment(RootComponent);

            // 상대 위치를 조정하여 옆면이 보이도록 설정
            FVector AdjustedOffset = FVector(Offset.Y, Offset.X, Offset.Z); // X와 Y를 교환
            NewMesh->SetRelativeLocation(AdjustedOffset);

            // 중력 및 물리 시뮬레이션 비활성화
            NewMesh->SetSimulatePhysics(false);
            NewMesh->SetEnableGravity(false);

            NewMesh->RegisterComponent();
            BlockMeshes.Add(NewMesh);
        }
    }
}

void ATetrisBlock::Move(const FVector& Offset)
{
    FVector NewLocation = GetActorLocation() + Offset;
    SetActorLocation(NewLocation);

    //UE_LOG(LogTemp, Warning, TEXT("Block moved to: %s"), *NewLocation.ToString());
}

void ATetrisBlock::Rotate()
{
    // 기존 위치 저장 (충돌 시 복구를 위해)
    TArray<FVector> OriginalLocations;
    for (UStaticMeshComponent* Mesh : BlockMeshes)
    {
        OriginalLocations.Add(Mesh->GetRelativeLocation());
    }

    // 블록의 중심 계산
    FVector BlockCenter = FVector::ZeroVector;
    for (UStaticMeshComponent* Mesh : BlockMeshes)
    {
        BlockCenter += Mesh->GetRelativeLocation();
    }
    BlockCenter /= BlockMeshes.Num(); // 중심점 계산

    // 각 큐브의 상대 위치를 기준으로 90도 회전
    for (UStaticMeshComponent* Mesh : BlockMeshes)
    {
        FVector RelativeLocation = Mesh->GetRelativeLocation() - BlockCenter;

        // 2D 회전 변환 (Z축은 고정)
        float NewX = -RelativeLocation.Y;
        float NewY = RelativeLocation.X;

        // 새로운 위치 설정 (중심점 기준으로 이동)
        Mesh->SetRelativeLocation(FVector(NewX, NewY, RelativeLocation.Z) + BlockCenter);
    }

    // 충돌 감지 (HasCollision 함수 필요)
    if (/* 충돌 감지 로직 */ false) // 충돌 감지 로직을 구현하세요
    {
        // 충돌 시 원래 위치로 복구
        for (int32 i = 0; i < BlockMeshes.Num(); i++)
        {
            BlockMeshes[i]->SetRelativeLocation(OriginalLocations[i]);
        }

        UE_LOG(LogTemp, Warning, TEXT("Rotation canceled due to collision."));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Block rotated successfully."));
    }
}

const TArray<UStaticMeshComponent*>& ATetrisBlock::GetBlockMeshes() const
{
    return BlockMeshes;
}