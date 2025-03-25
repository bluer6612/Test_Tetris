#pragma once

#include "CoreMinimal.h"
#include "../Blocks/TetrisBlock.h" // TetrisBlock 클래스의 경로
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

    // 블록 이동 및 회전 함수
    void MoveLeft();
    void MoveRight();
    void MoveDown();
    void RotateBlock();

private:
    void SpawnBlock(); // 블록 생성
    void ClearFullRows(); // 가득 찬 줄 제거
    bool HasCollision(const FVector& Location); // 충돌 감지 함수

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tetris")
    TSubclassOf<ATetrisBlock> BlockClass; // 블록 클래스 설정

private:
    ATetrisBlock* ActiveBlock; // 현재 활성 블록
};