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

    void MoveLeft();
    void MoveRight();
    void MoveDown();
    void Rotate();
};