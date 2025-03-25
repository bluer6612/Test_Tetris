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

    void InitializeBlock(const TArray<FVector>& BlockShape);
    void Rotate();
    void MoveLeft();
    void MoveRight();

private:
    UPROPERTY(VisibleAnywhere)
    UStaticMesh* CubeMesh; // 큐브 메쉬

    UPROPERTY(EditAnywhere)
    TArray<UStaticMeshComponent*> BlockMeshes; // 블록을 구성하는 큐브들
};