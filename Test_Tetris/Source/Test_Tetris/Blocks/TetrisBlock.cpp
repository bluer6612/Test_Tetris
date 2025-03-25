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
    FRotator NewRotation = GetActorRotation();
    NewRotation.Yaw += 90.0f; // 90도 회전
    SetActorRotation(NewRotation);

    UE_LOG(LogTemp, Warning, TEXT("Block rotated to: %s"), *NewRotation.ToString());
}

const TArray<UStaticMeshComponent*>& ATetrisBlock::GetBlockMeshes() const
{
    return BlockMeshes;
}