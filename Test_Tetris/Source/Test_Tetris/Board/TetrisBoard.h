#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TetrisBoard.generated.h"

// 블록 클래스 선언
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

public:
    // 블록 클래스를 설정할 수 있는 변수
    UPROPERTY(EditAnywhere, Category = "Tetris")
    TSubclassOf<ATetrisBlock> BlockClass;
};