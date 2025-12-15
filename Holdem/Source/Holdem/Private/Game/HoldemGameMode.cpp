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
		if(GS->GetPlayerCount() >= 1)
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

		// 1초 동안 Look 입력 비활성화
		/*
		GS->bIsLookDisabled = true;
		GetWorldTimerManager().SetTimer(LookDisableTimerHandle,[this, GS]()
			{
				if (GS)
				{
					GS->bIsLookDisabled = false;
				}
			},LookDisableTime, false);
		*/

		GS->GenerateDeck();
		GS->ShuffleDeck();
		
		GS->DealPreflopToPlayers();
		GS->SpawnPlayerItem();

		// Blind 자동 베팅
		// TODO: Dealer 포지션 시스템 구현 후 수정 필요
		// 현재는 임시로 0번 = Small Blind, 1번 = Big Blind
		if (GS->PlayerArray.Num() >= 2)
		{
			// Small Blind (0번 플레이어)
			if (AHoldemPlayerState* SB = Cast<AHoldemPlayerState>(GS->PlayerArray[0]))
			{
				SB->CurrentChips -= GS->SmallBlindAmount;
				SB->CurrentBet = GS->SmallBlindAmount;
				GS->CurrentMaxBet = GS->SmallBlindAmount;
				UE_LOG(LogTemp, Warning, TEXT("[Blind] %s posted Small Blind: %d"),
					*SB->GetPlayerName(), GS->SmallBlindAmount);
			}

			// Big Blind (1번 플레이어)
			if (AHoldemPlayerState* BB = Cast<AHoldemPlayerState>(GS->PlayerArray[1]))
			{
				BB->CurrentChips -= GS->BigBlindAmount;
				BB->CurrentBet = GS->BigBlindAmount;
				GS->CurrentMaxBet = GS->BigBlindAmount;
				UE_LOG(LogTemp, Warning, TEXT("[Blind] %s posted Big Blind: %d"),
					*BB->GetPlayerName(), GS->BigBlindAmount);
			}
		}
		
		// 베팅 시작
		StartBettingRound();
		
		// Testing
		// GetWorldTimerManager().SetTimer(TestingTimer,
		// 	this, &AHoldemGameMode::StartFlop, PhaseDelay, false);
	}
}

void AHoldemGameMode::StartFlop()
{
	AHoldemGameState* GS = GetGameState<AHoldemGameState>();
	if (GS)
	{
		ChangeGamePhase(EHoldemPhase::Flop);
		GS->DealFlopCards();

		StartBettingRound();
		
		// // Testing
		// GetWorldTimerManager().SetTimer(TestingTimer,
		// 	this, &AHoldemGameMode::StartTurn, PhaseDelay, false);
	}
}

void AHoldemGameMode::StartTurn()
{
	AHoldemGameState* GS = GetGameState<AHoldemGameState>();
	if (GS)
	{
		ChangeGamePhase(EHoldemPhase::Turn);
		GS->DealTurnCard();

		StartBettingRound();

		// // Testing
		// GetWorldTimerManager().SetTimer(TestingTimer,
		// 	this, &AHoldemGameMode::StartRiver, PhaseDelay, false);
	}
}

void AHoldemGameMode::StartRiver()
{
	AHoldemGameState* GS = GetGameState<AHoldemGameState>();
	if (GS)
	{
		ChangeGamePhase(EHoldemPhase::River);
		GS->DealRiverCard();

		StartBettingRound();
		
		// // Testing
		// GetWorldTimerManager().SetTimer(TestingTimer,
		// 	this, &AHoldemGameMode::StartShowdown, PhaseDelay, false);
	}
}

void AHoldemGameMode::StartShowdown()
{
	AHoldemGameState* GS = GetGameState<AHoldemGameState>();
	if (GS)
	{
		ChangeGamePhase(EHoldemPhase::Showdown);

		TArray<APlayerState*> ActivePlayers;
		for (auto PS : GS->PlayerArray)
		{
			AHoldemPlayerState* Player = Cast<AHoldemPlayerState>(PS);
			if (Player && !Player->bIsFolded)
				ActivePlayers.Add(Player);
		}

		// 승자 판정 
		TArray<APlayerState*> Winners = GS->DetermineWinner(ActivePlayers);

		// 최종 결과 로그
		if (Winners.Num() > 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("========== SHOWDOWN RESULT =========="));

			if (Winners.Num() == 1)
			{
				// 단독 승자                                                                                                                            
				AHoldemPlayerState* Winner = Cast<AHoldemPlayerState>(Winners[0]);
				if (Winner)
				{
					UE_LOG(LogTemp, Warning, TEXT("🏆 Winner: %s"), *Winner->GetPlayerName());
				}
			}
			else                                                                                                                                            
			{
				// 동점 승자들                                                                                                                          
				UE_LOG(LogTemp, Warning, TEXT("🤝 Split Pot! Winners:"));
				for (APlayerState* WinnerPS : Winners)
				{
					AHoldemPlayerState* Winner = Cast<AHoldemPlayerState>(WinnerPS);
					if (Winner)
					{
						UE_LOG(LogTemp, Warning, TEXT("  - %s"), *Winner->GetPlayerName());
					}
				}
			}

			UE_LOG(LogTemp, Warning, TEXT("====================================="));
		}
		else                                                                                                                                                    
		{
			UE_LOG(LogTemp, Error, TEXT("[StartShowdown] No winners found!"));
		}
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

void AHoldemGameMode::StartBettingRound()
{
	AHoldemGameState* GS = GetGameState<AHoldemGameState>();
	if (!GS) return;

	// 모든 플레이어 플래그 리셋
	for (APlayerState* PS : GS->PlayerArray)
	{
		AHoldemPlayerState* Player = Cast<AHoldemPlayerState>(PS);
		if (Player)
		{
			Player->bHasActedThisRound = false;
		}
	}

	// 첫번째 턴 플레이어 설정
	// (임시로 0번부터 시작. 추후에 Dealer/Blind 적용 예정)
	GS->CurrentTurnPlayerIndex = 0;

	// Fold한 플레이어 스킵
	while (GS->CurrentTurnPlayerIndex < GS->PlayerArray.Num())
	{
		AHoldemPlayerState* Player = Cast<AHoldemPlayerState>(
			GS->PlayerArray[GS->CurrentTurnPlayerIndex]);

		if (Player && !Player->bIsFolded) break;

		GS->CurrentTurnPlayerIndex++;
	}

	UE_LOG(LogTemp, Warning, TEXT("[StartBettingRound] Betting round started. First player: %d"),
			   GS->CurrentTurnPlayerIndex);

	GS->OnTurnChanged.Broadcast(GS->CurrentTurnPlayerIndex);
}

bool AHoldemGameMode::IsBettingRoundComplete()
{
	AHoldemGameState* GS = GetGameState<AHoldemGameState>();
	if (!GS) return false;

	// ActivePlayer 카운트
	int32 ActivePlayers = 0;
	
	for (APlayerState* PS : GS->PlayerArray)
	{
		AHoldemPlayerState* Player = Cast<AHoldemPlayerState>(PS);
		if (Player && !Player->bIsFolded)
		{
			ActivePlayers++;

			// 조건 1 : 액션하지 않은 플레이어 있으면
			if (!Player->bHasActedThisRound) return false;
			// 조건 2 : 베팅액이 최고액과 다르면 아직 안끝남
			if (Player->CurrentBet != GS->CurrentMaxBet) return false;
		}
	}

	// 1명만 남기고 모두 Fold한 경우, 라운드 종료
	if (ActivePlayers <= 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("[IsBettingRoundComplete] Only 1 active player left!"));
		return true;
	}

	// 모든 조건 만족 → 베팅 라운드 완료!
	UE_LOG(LogTemp, Warning, TEXT("[IsBettingRoundComplete] Betting round complete!"));
	return true;
}

void AHoldemGameMode::MoveToNextPlayer()
{
	AHoldemGameState* GS = GetGameState<AHoldemGameState>();
	if (!GS) return;

	if (IsBettingRoundComplete())
	{
		GS->CollectBetsIntoPot();
		GS->CurrentTurnPlayerIndex = -1;
		//
		GS->OnTurnChanged.Broadcast(-1);
		
		// 다음 Phase로 전환
		switch (GS->CurrentPhase)
		{
		case EHoldemPhase::PreFlop:
			StartFlop();
			break;

		case EHoldemPhase::Flop:
			StartTurn();
			break;

		case EHoldemPhase::Turn:
			StartRiver();
			break;

		case EHoldemPhase::River:
			StartShowdown();
			break;

		default:
			UE_LOG(LogTemp, Warning, TEXT("[MoveToNextPlayer] Unknown phase!"));
			break;
		}

		return;
	}

	// 다음 플레이어 찾기
	int32 StartIndex = GS->CurrentTurnPlayerIndex;
	int32 PlayerCount = GS->PlayerArray.Num();

	do
	{
		// 다음 인덱스로 이동
		GS->CurrentTurnPlayerIndex = (GS->CurrentTurnPlayerIndex + 1) % PlayerCount;

		AHoldemPlayerState* NextPlayer = Cast<AHoldemPlayerState>(
			GS->PlayerArray[GS->CurrentTurnPlayerIndex]);

		// Fold하지 않은 플레이어 찾으면 종료
		if (NextPlayer && !NextPlayer->bIsFolded)
		{
			UE_LOG(LogTemp, Warning, TEXT("[MoveToNextPlayer] Next turn: Player %d (%s)"),
								GS->CurrentTurnPlayerIndex, *NextPlayer->GetPlayerName());

			GS->OnTurnChanged.Broadcast(GS->CurrentTurnPlayerIndex);
			
			return;
		}

		// 무한 루프 방지
		if (GS->CurrentTurnPlayerIndex == StartIndex)
		{
			UE_LOG(LogTemp, Error, TEXT("[MoveToNextPlayer] No valid player found!"));
			return;
		}
	}while (true);
}
