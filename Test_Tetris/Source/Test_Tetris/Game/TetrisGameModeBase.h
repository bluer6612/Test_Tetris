#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TetrisGameModeBase.generated.h"

/**
 * Game Mode Base for Tetris
 */
UCLASS()
class TEST_TETRIS_API ATetrisGameModeBase : public AGameModeBase
{
    GENERATED_BODY()

public:
    ATetrisGameModeBase();

protected:
    virtual void BeginPlay() override;

private:
    void StartGame();
    void EndGame();
};