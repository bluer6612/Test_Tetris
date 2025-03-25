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

    // 입력 이벤트 처리 함수
    void MoveLeft();
    void MoveRight();
    void MoveDown();
    void RotateBlock();

private:
    void SpawnBlock(); // 블록 생성
    void ClearFullRows(); // 가득 찬 줄 제거
    bool HasCollision(const FVector& Location); // 충돌 감지 함수

    // 테트리스 보드 크기 정의
    static const int BoardWidth = 10;  // 보드의 가로 크기 (10칸)
    static const int BoardHeight = 20; // 보드의 세로 크기 (20칸)

    // 보드 상태를 저장하는 2D 배열
    bool Board[BoardWidth][BoardHeight] = { false };

    // 테트리스 블록 모양 정의
    TArray<FVector> IBlock = {
        FVector(0, 0, 0),
        FVector(100, 0, 0),
        FVector(-100, 0, 0),
        FVector(-200, 0, 0)
    };

    TArray<FVector> TBlock = {
        FVector(0, 0, 0),
        FVector(100, 0, 0),
        FVector(-100, 0, 0),
        FVector(0, 0, 100)
    };

    TArray<FVector> LBlock = {
        FVector(0, 0, 0),
        FVector(100, 0, 0),
        FVector(-100, 0, 0),
        FVector(-100, 0, 100)
    };

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tetris")
    TSubclassOf<ATetrisBlock> BlockClass; // 블록 클래스 설정

private:
    ATetrisBlock* ActiveBlock; // 현재 활성 블록
    bool bIsGameOver = false; // 게임 오버 상태
    float BlockFallInterval = 3.0f; // 블록이 내려오는 간격 (초 단위)
    float TimeSinceLastFall = 1.0f; // 마지막으로 블록이 내려온 이후 경과 시간
};