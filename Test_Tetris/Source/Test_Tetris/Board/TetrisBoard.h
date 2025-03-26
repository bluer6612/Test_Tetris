#pragma once

#include "CoreMinimal.h"
#include "../Blocks/TetrisBlock.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/Material.h"
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

    // 경계 프레임 생성 함수
    void CreateBorderFrames();

    // 기본 스케일
    FVector DefaultScale;

    // 경계 프레임을 위한 메시 컴포넌트
    UPROPERTY()
    TArray<UStaticMeshComponent*> BorderFrames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tetris")
    TSubclassOf<ATetrisBlock> BlockClass; // 블록 클래스 설정

    UFUNCTION(BlueprintCallable, Category="Tetris")
    bool IsBlockTouchingGround(class ATetrisBlock* Block);

public:
    virtual void Tick(float DeltaTime) override;

    // 입력 이벤트 처리 함수
    void MoveLeft();
    void MoveRight();
    void MoveDown();
    void RotateBlock();
    void HardDrop();

    // 블록을 바닥으로부터 50.0f 위에 위치시키는 함수 선언
    void AdjustBlockAboveGround(class ATetrisBlock* Block);

    // 테트리스 보드 크기 정의
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tetris Board")
    int BoardWidth = 10;  // 보드의 가로 크기 (10칸)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tetris Board")
    int BoardHeight = 20; // 보드의 세로 크기 (20칸)

private:
    void SpawnBlock(); // 블록 생성
    void ClearFullRows(); // 가득 찬 줄 제거
    bool HasCollision(const FVector& Location); // 충돌 감지 함수

    // 보드 상태를 저장하는 2D 배열
    TArray<TArray<bool>> Board;

    // 테트리스 블록 모양 정의
    TArray<FVector> IBlock = {
        FVector(0, 0, 0),
        FVector(0, 0, 100),
        FVector(0, 0, 200),
        FVector(0, 0, 300)
    };

    TArray<FVector> OBlock = {
        FVector(0, 0, 0),
        FVector(100, 0, 0),
        FVector(0, 0, 100),
        FVector(100, 0, 100)
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

    TArray<FVector> JBlock = {
        FVector(0, 0, 0),
        FVector(100, 0, 0),
        FVector(-100, 0, 0),
        FVector(100, 0, 100)
    };

    TArray<FVector> SBlock = {
        FVector(0, 0, 0),
        FVector(100, 0, 0),
        FVector(0, 0, 100),
        FVector(-100, 0, 100)
    };

    TArray<FVector> ZBlock = {
        FVector(0, 0, 0),
        FVector(-100, 0, 0),
        FVector(0, 0, 100),
        FVector(100, 0, 100)
    };

    ATetrisBlock* ActiveBlock; // 현재 활성 블록
    bool bIsGameOver = false; // 게임 오버 상태
};