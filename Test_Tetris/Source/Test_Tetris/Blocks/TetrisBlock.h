#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TetrisBlock.generated.h"

UCLASS()
class TEST_TETRIS_API ATetrisBlock : public AActor
{
    GENERATED_BODY()

public:
    ATetrisBlock();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // 블록 초기화 함수
    void InitializeBlock(const TArray<FVector>& BlockShape);

    // 블록 동작 함수
    void Move(const FVector& Offset);
    void Rotate();

    // BlockMeshes에 접근할 수 있는 Getter 함수
    const TArray<UStaticMeshComponent*>& GetBlockMeshes() const;

private:
    UPROPERTY(VisibleAnywhere)
    UStaticMesh* CubeMesh; // 큐브 메쉬

    UPROPERTY(EditAnywhere)
    TArray<UStaticMeshComponent*> BlockMeshes; // 블록을 구성하는 큐브들

    int32 RotationState = 0; // 현재 회전 상태 (0: 0도, 1: 90도, 2: 180도, 3: 270도)
};