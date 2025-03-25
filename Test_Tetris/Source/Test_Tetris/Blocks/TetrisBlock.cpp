#include "TetrisBlock.h"
#include "GameFramework/PlayerController.h"

ATetrisBlock::ATetrisBlock()
{
    PrimaryActorTick.bCanEverTick = true;

    // Pawn이 입력을 받을 수 있도록 AutoPossessPlayer 설정
    AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void ATetrisBlock::BeginPlay()
{
    Super::BeginPlay();
}

void ATetrisBlock::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ATetrisBlock::MoveDown()
{
    FVector NewLocation = GetActorLocation();
    NewLocation.Z -= 100.0f; // 아래로 이동
    SetActorLocation(NewLocation);

    UE_LOG(LogTemp, Warning, TEXT("Block moved down to: %s"), *NewLocation.ToString());
}

void ATetrisBlock::Rotate()
{
    FRotator NewRotation = GetActorRotation();
    NewRotation.Yaw += 90.0f; // 90도 회전
    SetActorRotation(NewRotation);

    UE_LOG(LogTemp, Warning, TEXT("Block rotated to: %s"), *NewRotation.ToString());
}

void ATetrisBlock::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAction("MoveLeft", IE_Pressed, this, &ATetrisBlock::MoveLeft);
    PlayerInputComponent->BindAction("MoveRight", IE_Pressed, this, &ATetrisBlock::MoveRight);
    PlayerInputComponent->BindAction("MoveDown", IE_Pressed, this, &ATetrisBlock::MoveDown);
    PlayerInputComponent->BindAction("Rotate", IE_Pressed, this, &ATetrisBlock::Rotate);
}

void ATetrisBlock::MoveLeft()
{
    FVector NewLocation = GetActorLocation();
    NewLocation.X -= 100.0f; // 왼쪽으로 이동
    SetActorLocation(NewLocation);

    UE_LOG(LogTemp, Warning, TEXT("Block moved left to: %s"), *NewLocation.ToString());
}

void ATetrisBlock::MoveRight()
{
    FVector NewLocation = GetActorLocation();
    NewLocation.X += 100.0f; // 오른쪽으로 이동
    SetActorLocation(NewLocation);

    UE_LOG(LogTemp, Warning, TEXT("Block moved right to: %s"), *NewLocation.ToString());
}