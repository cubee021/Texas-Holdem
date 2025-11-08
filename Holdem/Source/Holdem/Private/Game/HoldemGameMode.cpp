// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/HoldemGameMode.h"

#include "MyPlayer.h"
#include "MyPlayerController.h"
#include "Game/HoldemGameState.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

AHoldemGameMode::AHoldemGameMode()
{
	DefaultPawnClass = AMyPlayer::StaticClass();
	PlayerControllerClass = AMyPlayerController::StaticClass();
	GameStateClass = AHoldemGameState::StaticClass();
}

void AHoldemGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// 플레이어 수 확인
	AHoldemGameState* GS = GetGameState<AHoldemGameState>();
	if (GS)
	{
		int32 PlayerCount = GS->GetPlayerCount();
		UE_LOG(LogTemp, Log, TEXT("Player connected! Total players: %d/%d"), PlayerCount, GS->MaxPlayers);
		UE_LOG(LogTemp, Log, TEXT("Player Name: %s"), *NewPlayer->GetName());
	}
}

AActor* AHoldemGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	// 접속한 순서대로 인덱스 배정
	int32 PlayerIdx = 0;

	AHoldemGameState* GS = GetGameState<AHoldemGameState>();
	if (GS)
	{
		PlayerIdx = GS->GetPlayerCount();
	}

	// Tag로 배정할 PlayerStart 찾기
	FString SeatTag = FString::Printf(TEXT("Seat_%d"), PlayerIdx);

	TArray<AActor*> AllPlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), AllPlayerStarts);

	for (AActor* PlayerStart : AllPlayerStarts)
	{
		if (PlayerStart->Tags.Contains(*SeatTag))
		{
			UE_LOG(LogTemp, Log, TEXT("Player %d assigned to %s"), PlayerIdx, *SeatTag);
			return PlayerStart;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Seat_%d not found!"), PlayerIdx);
	return Super::ChoosePlayerStart_Implementation(Player);
}
