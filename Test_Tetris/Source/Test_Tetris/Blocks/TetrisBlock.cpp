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

    FVector NewLocation = GetActorLocation();
    NewLocation.Z -= 100.0f; // 아래로 이동
    SetActorLocation(NewLocation);
    UE_LOG(LogTemp, Warning, TEXT("Block moved down to: %s"), *NewLocation.ToString());
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
            NewMesh->RegisterComponent();
            BlockMeshes.Add(NewMesh);
        }
    }
}

void ATetrisBlock::Rotate()
{
    FRotator NewRotation = GetActorRotation();
    NewRotation.Yaw += 90.0f; // 90도 회전
    SetActorRotation(NewRotation);

    UE_LOG(LogTemp, Warning, TEXT("Block rotated to: %s"), *NewRotation.ToString());
}

void ATetrisBlock::MoveLeft()
{
    FVector NewLocation = GetActorLocation();
    NewLocation.X -= 100.0f; // 왼쪽으로 이동
    SetActorLocation(NewLocation);

    UE_LOG(LogTemp, Warning, TEXT("Block moved left to: %s"), *NewLocation.ToString());
}

void ATetrisBlock::MoveRight()
{
    FVector NewLocation = GetActorLocation();
    NewLocation.X += 100.0f; // 오른쪽으로 이동
    SetActorLocation(NewLocation);

    UE_LOG(LogTemp, Warning, TEXT("Block moved right to: %s"), *NewLocation.ToString());
}