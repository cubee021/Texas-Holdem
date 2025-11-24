// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/HoldemGameMode.h"

#include "MyPlayer.h"
#include "MyPlayerController.h"
#include "Game/HoldemGameState.h"
#include "Game/HoldemPlayerState.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

AHoldemGameMode::AHoldemGameMode()
{
	DefaultPawnClass = AMyPlayer::StaticClass();
	PlayerControllerClass = AMyPlayerController::StaticClass();
	GameStateClass = AHoldemGameState::StaticClass();
	PlayerStateClass = AHoldemPlayerState::StaticClass();
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

void AHoldemGameMode::BeginPlay()
{
	Super::BeginPlay();

	StartWaiting();
}

void AHoldemGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AHoldemGameMode::StartWaiting()
{
	AHoldemGameState* GS = GetGameState<AHoldemGameState>();
	if (GS)
	{
		ChangeGamePhase(EHoldemPhase::Waiting);
		GS->WaitingTimeRemaining = GS->WaitingDuration;

		GetWorldTimerManager().SetTimer(WaitingTimerHandle, this,
			&AHoldemGameMode::UpdateWaitingTimer, 1.f, true);
	}
}

void AHoldemGameMode::UpdateWaitingTimer()
{
	AHoldemGameState* GS = GetGameState<AHoldemGameState>();
	if (GS)
	{
		GS->WaitingTimeRemaining -= 1.f;
	}

	if (GS->WaitingTimeRemaining <= 0.f)
	{
		if(GS->GetPlayerCount() >= 2)
		{
			// 두 명 이상이면 게임 진행
			GetWorldTimerManager().ClearTimer(WaitingTimerHandle);
			StartPreFlop();
		}
		else
		{
			// 한 명밖에 없으면 다시 대기
			StartWaiting();
		}
	}
}

void AHoldemGameMode::StartPreFlop()
{
	AHoldemGameState* GS = GetGameState<AHoldemGameState>();
	if (GS)
	{
		ChangeGamePhase(EHoldemPhase::PreFlop);

		GS->GenerateDeck();
		GS->ShuffleDeck();

		GS->SpawnPlayerItem();
		//GS->DealPreflopToPlayers();
	}
}

void AHoldemGameMode::StartFlop()
{
	AHoldemGameState* GS = GetGameState<AHoldemGameState>();
	if (GS)
	{
		GS->DealFlopCards();
	}
}

void AHoldemGameMode::StartTurn()
{
	AHoldemGameState* GS = GetGameState<AHoldemGameState>();
	if (GS)
	{
		GS->DealTurnCard();
	}
}

void AHoldemGameMode::StartRiver()
{
	AHoldemGameState* GS = GetGameState<AHoldemGameState>();
	if (GS)
	{
		GS->DealRiverCard();
	}
}

void AHoldemGameMode::ChangeGamePhase(EHoldemPhase NewState)
{
	AHoldemGameState* GS = GetGameState<AHoldemGameState>();
	if (!GS) return;
	
	// 클라이언트쪽 상태 변경
	GS->CurrentPhase = NewState;

	// 서버쪽 상태 변경 브로드캐스트
	if (HasAuthority())
	{
		GS->OnPhaseChanged.Broadcast(NewState);
		GS->PreviousPhase = NewState;
	}
}
