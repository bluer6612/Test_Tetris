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
            NewMesh->SetRelativeLocation(Offset);

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

    UE_LOG(LogTemp, Warning, TEXT("Block moved to: %s"), *NewLocation.ToString());
}

void ATetrisBlock::Rotate()
{
    // 기존 위치 저장 (충돌 시 복구를 위해)
    TArray<FVector> OriginalLocations;
    for (UStaticMeshComponent* Mesh : BlockMeshes)
    {
        OriginalLocations.Add(Mesh->GetRelativeLocation());
    }

    // 회전 상태 업데이트 (0 -> 1 -> 2 -> 3 -> 0)
    RotationState = (RotationState + 1) % 4;

    // 각 회전 상태에 따른 상대 위치 계산
    for (int32 i = 0; i < BlockMeshes.Num(); i++)
    {
        FVector NewRelativeLocation;

        switch (RotationState)
        {
        case 0: // 0도
            NewRelativeLocation = OriginalLocations[i];
            break;
        case 1: // 90도
            NewRelativeLocation = FVector(-OriginalLocations[i].Y, OriginalLocations[i].X, OriginalLocations[i].Z);
            break;
        case 2: // 180도
            NewRelativeLocation = FVector(-OriginalLocations[i].X, -OriginalLocations[i].Y, OriginalLocations[i].Z);
            break;
        case 3: // 270도
            NewRelativeLocation = FVector(OriginalLocations[i].Y, -OriginalLocations[i].X, OriginalLocations[i].Z);
            break;
        }

        // 새로운 상대 위치 설정
        BlockMeshes[i]->SetRelativeLocation(NewRelativeLocation);
    }

    // 충돌 감지 (HasCollision 함수 필요)
    if (/* 충돌 감지 로직 */ false) // 충돌 감지 로직을 구현하세요
    {
        // 충돌 시 원래 위치로 복구
        for (int32 i = 0; i < BlockMeshes.Num(); i++)
        {
            BlockMeshes[i]->SetRelativeLocation(OriginalLocations[i]);
        }

        // 회전 상태 복구
        RotationState = (RotationState - 1 + 4) % 4;

        UE_LOG(LogTemp, Warning, TEXT("Rotation canceled due to collision."));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Block rotated to state: %d"), RotationState);
    }
}

const TArray<UStaticMeshComponent*>& ATetrisBlock::GetBlockMeshes() const
{
    return BlockMeshes;
}