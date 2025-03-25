#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TetrisBoard.generated.h"

class ATetrisBlock;

UCLASS()
class TEST_TETRIS_API ATetrisBoard : public AActor
{
    GENERATED_BODY()

public:
    ATetrisBoard();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

private:
    void SpawnBlock();
    void ClearFullRows();
    bool HasCollision(const FVector& Location); // 충돌 감지 함수

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tetris")
    TSubclassOf<ATetrisBlock> BlockClass;

private:
    ATetrisBlock* ActiveBlock; // 현재 활성 블록
};