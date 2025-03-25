#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h" // AActor 대신 APawn을 상속받기 위해 변경
#include "TetrisBlock.generated.h"

/**
 * Represents a single Tetris block
 */
UCLASS()
class TEST_TETRIS_API ATetrisBlock : public APawn // APawn으로 변경
{
    GENERATED_BODY()

public:
    ATetrisBlock();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // 블록 이동 및 회전 함수
    void MoveLeft();
    void MoveRight();
    void MoveDown();
    void Rotate();

    // 입력 컴포넌트 설정
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
};