// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/HoldemGameState.h"

#include "Game/HandEvaluator.h"
#include "Game/HoldemPlayerState.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

AHoldemGameState::AHoldemGameState()
{
	MaxPlayers = 4;
	CurrentPhase = EHoldemPhase::Waiting;
	PreviousPhase = EHoldemPhase::Waiting;
	bIsLookDisabled = false;
}

void AHoldemGameState::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		HandEvaluator = NewObject<UHandEvaluator>(this);
	}
}

void AHoldemGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHoldemGameState, SpawnedCards);
	DOREPLIFETIME(AHoldemGameState, CurrentPhase);
	DOREPLIFETIME(AHoldemGameState, CommunityCards);
	DOREPLIFETIME(AHoldemGameState, WaitingTimeRemaining);
	DOREPLIFETIME(AHoldemGameState, bIsLookDisabled);

	DOREPLIFETIME(AHoldemGameState, TotalPot);
	DOREPLIFETIME(AHoldemGameState, CurrentMaxBet);
	DOREPLIFETIME(AHoldemGameState, CurrentTurnPlayerIndex);
	
	DOREPLIFETIME(AHoldemGameState, DealerPosition);
}

void AHoldemGameState::GenerateDeck()
{
	if (!HasAuthority()) return;

	Deck.Empty();
	// 52장의 카드 덱에 추가
	for (int32 SuitIdx = 0; SuitIdx < 4; SuitIdx++)
	{
		for (int32 RankIdx = 0; RankIdx < 13; RankIdx++)
		{
			FCardData NewCard;
			
			NewCard.Suit = (ECardSuit)SuitIdx;
			NewCard.Rank = (ECardRank)RankIdx;

			Deck.Add(NewCard);
		}
	}
}

void AHoldemGameState::ShuffleDeck()
{
	if (!HasAuthority()) return;

	// Fisher-Yates 셔플 알고리즘 :
	//		마지막 요소부터 시작해서 각 요소를 랜덤하게 선택된 앞쪽 요소와 위치를 교환
	//		(편향되지 않고 잘 섞이는 알고리즘이라고 함)
	for (int32 i=Deck.Num()-1; i>0; i--)
	{
		int32 j = FMath::RandRange(0, i);
		Deck.Swap(i, j);
	}
}

ACard* AHoldemGameState::SpawnCard(const FCardData& Data, FVector Location, FRotator Rotation)
{
	if (!HasAuthority()) return nullptr;
	if (!CardClass) return nullptr;

	// 카드 스폰
	// AlwaysSpawn : spawn 보장 (다른 카드나 테이블에 물리 충돌로 튕겨나갈 수 있음)
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ACard* NewCard = GetWorld()->SpawnActor<ACard>(CardClass, Location, Rotation, SpawnParams);
	if (NewCard)
	{
		// 스폰 후 데이터 설정
		NewCard->SetCardData(Data);

		// 현재 위치 origin으로 설정
		NewCard->OriginalPosition = Location;
		NewCard->OriginalRotation = Rotation;

		NewCard->CardState = ECardState::OnTable;

		// 스폰된 카드 목록에 추가
		SpawnedCards.Add(NewCard);
	}

	return NewCard;
}

ACard* AHoldemGameState::DrawAndSpawnCard(FVector Location, FRotator Rotation)
{
	if (!HasAuthority()) return nullptr;
	if (Deck.Num() == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Deck empty"));
		return nullptr;
	}

	FCardData DrawCard = Deck.Pop();
	
	return SpawnCard(DrawCard, Location, Rotation);
}

void AHoldemGameState::ResetAllCardsLocation()
{
	if (!HasAuthority()) return;

	//for (ACard)
}

void AHoldemGameState::OnRep_CurrentPhase()
{
	if (CurrentPhase != PreviousPhase)
	{
		UE_LOG(LogTemp, Warning, TEXT("[HoldemGameState] Phase changed from %s to %s"),
			*UEnum::GetValueAsString(PreviousPhase), *UEnum::GetValueAsString(CurrentPhase));

		OnPhaseChanged.Broadcast(CurrentPhase);
		PreviousPhase = CurrentPhase;
	}
}

void AHoldemGameState::DealPreflopToPlayers()
{
	if (!HasAuthority()) return;

	// 모든 플레이어에게 카드 배분
	for (APlayerState* PlayerState : PlayerArray)
	{
		AHoldemPlayerState* PS = Cast<AHoldemPlayerState>(PlayerState);
		if (PS)
		{
			PS->ClearHand();

			FVector FirstCardLoc, SecondCardLoc;
			GetPlayerCardSpawnLocation(PS, FirstCardLoc, SecondCardLoc);

			// ★ 카드 회전: 테이블 중심 → 플레이어 방향 기준
			APawn* Player = PS->GetPawn();
			FVector ForwardDirection = GetForwardDirection(Player);

			// Forward 방향을 향하도록 회전 행렬 생성
			FRotator CardRotation = FRotationMatrix::MakeFromYZ(ForwardDirection, FVector::UpVector).Rotator();

			ACard* FirstCard = DrawAndSpawnCard(FirstCardLoc, CardRotation);
			if (FirstCard)
			{
				// 덱에서 뽑은 카드 플레이어에게 추가
				PS->AddCard(FirstCard);
				FirstCard->CardState = ECardState::InHand;
			}

			ACard* SecondCard = DrawAndSpawnCard(SecondCardLoc, CardRotation);
			if (SecondCard)
			{
				// 덱에서 뽑은 카드 플레이어에게 추가
				PS->AddCard(SecondCard);
				SecondCard->CardState = ECardState::InHand;
			}
		}
	}
}

void AHoldemGameState::DealFlopCards()
{
	if (!HasAuthority()) return;

	for (int32 i=0; i<3; i++)
	{
		SpawnCommunityCard(i, CommunityCardRotationAngle);
	}
}

void AHoldemGameState::DealTurnCard()
{
	if (!HasAuthority()) return;

	SpawnCommunityCard(3, CommunityCardRotationAngle);
}

void AHoldemGameState::DealRiverCard()
{
	if (!HasAuthority()) return;

	SpawnCommunityCard(4, CommunityCardRotationAngle);
}

void AHoldemGameState::GetPlayerCardSpawnLocation(APlayerState* PlayerState, FVector& OutFirstCardLoc,
                                                  FVector& OutSecondCardLoc)
{
	if (!PlayerState) return;
	
	APawn* Player = PlayerState->GetPawn();
	
	FVector ForwardDirection = GetForwardDirection(Player);
	FVector BaseLocation = Player->GetActorLocation() + (ForwardDirection * PlayerCardSpawnDistance);

	// ★ Forward와 직교하는 Right 벡터: Z축과 Forward의 외적
	FVector RightDirection = FVector::CrossProduct(FVector::UpVector, ForwardDirection);

	// BaseLocation 기준으로 좌우에 나란히 배치
	float HalfSpacing = PlayerCardSpacing / 2.f;

	OutFirstCardLoc = BaseLocation - (RightDirection * HalfSpacing);
	OutSecondCardLoc = BaseLocation + (RightDirection * HalfSpacing);
}

FVector AHoldemGameState::GetForwardDirection(AActor* Player)
{
	FVector PlayerLocation = Player->GetActorLocation();

	// ★ 핵심: 테이블 중심 → 플레이어 방향 = 플레이어의 "앞"
	// Z축 무시하고 평면상 방향만 계산
	return (TableLocation-PlayerLocation).GetSafeNormal2D();
}

void AHoldemGameState::SpawnCommunityCard(int32 CardIdx, float RotationAngle)
{
	// 기본 위치 계산 (가로 배열)
	float Offset = (CardIdx - 2) * CommunityCardSpacing;
	FVector LocalPos = FVector(Offset, 0, 0);

	// ★ 회전 적용: Z축 기준으로 배열 회전
	FVector RotatedPos = LocalPos.RotateAngleAxis(RotationAngle, FVector::UpVector);
	FVector SpawnLocation = TableLocation + RotatedPos;
	SpawnLocation.Z = TableHeight;

	// 카드 자체도 같은 각도로 회전
	FRotator CardRotation = FRotator(0.0f, RotationAngle, 0.0f);

	ACard* NewCard = DrawAndSpawnCard(SpawnLocation, CardRotation);
	if (NewCard)
	{
		CommunityCards.Add(NewCard);
		NewCard->CardState = ECardState::OnTable;

		NewCard->PlayFlipAnimation();
	}
}

TArray<APlayerState*> AHoldemGameState::DetermineWinner(const TArray<APlayerState*>& ActivePlayers)
{
	TArray<APlayerState*> Winners;

	TArray<FPlayerHandPair> PlayerHandPairs;
	for (auto PS : ActivePlayers)
	{
		if (AHoldemPlayerState* Player = Cast<AHoldemPlayerState>(PS))
		{
			// 해당 플레이어의 카드 + 공개 카드
			TArray<ACard*> AllCards;
			AllCards.Append(Player->HandCards);
			AllCards.Append(CommunityCards);

			// HandEvaluator에 7장 통째로 전달
			FHandEvaluation Result = HandEvaluator->GetBestHandFrom7Cards(AllCards);

			PlayerHandPairs.Add(FPlayerHandPair(Player, Result));

			UE_LOG(LogTemp, Warning, TEXT("[DetermineWinner] Player %s: Rank %s"),
								*Player->GetPlayerName(), *UEnum::GetValueAsString(Result.Rank));
		}
	}

	// 플레이어 없으면 빈 배열 반환
	if (PlayerHandPairs.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DetermineWinner] No active players!"));
		return Winners;
	}

	// High hand 찾기
	FHandEvaluation BestHand = PlayerHandPairs[0].Evaluate;
	for (const FPlayerHandPair& Pair : PlayerHandPairs)
	{
		if (Pair.Evaluate > BestHand) BestHand = Pair.Evaluate;
	}

	// High hand 가진 플레이어들 수집
	for (const FPlayerHandPair& Pair : PlayerHandPairs)
	{
		bool bIsSame = !(Pair.Evaluate > BestHand) && !(BestHand > Pair.Evaluate);

		if (bIsSame)
		{
			Winners.Add(Pair.Player);

			// 승자 로그                                                                                                                                    
			AHoldemPlayerState* WinnerPS = Cast<AHoldemPlayerState>(Pair.Player);
			if (WinnerPS)
			{
				UE_LOG(LogTemp, Warning, TEXT("[DetermineWinner] Winner: %s with Rank %s"),
						*WinnerPS->GetPlayerName(), *UEnum::GetValueAsString(Pair.Evaluate.Rank));
			}
		}
	}
	return Winners;
}

void AHoldemGameState::OnRep_CurrentTurnPlayerIndex()
{
	OnTurnChanged.Broadcast(CurrentTurnPlayerIndex);
}

void AHoldemGameState::ProcessFold(AHoldemPlayerState* Player)
{
	if (!HasAuthority() || !Player) return;

	Player->bIsFolded = true;
	Player->bHasActedThisRound = true;

	UE_LOG(LogTemp, Warning, TEXT("[Betting Log] %s folded"), *Player->GetPlayerName());
}

void AHoldemGameState::ProcessCheck(AHoldemPlayerState* Player)
{
	if (!HasAuthority() || !Player) return;

	Player->bHasActedThisRound = true;

	UE_LOG(LogTemp, Warning, TEXT("[Betting Log] %s checked"), *Player->GetPlayerName());
}

void AHoldemGameState::ProcessCall(AHoldemPlayerState* Player)
{
	if (!HasAuthority() || !Player) return;
	// 추가로 내야 할 금액
	int32 CallAmount = CurrentMaxBet - Player->CurrentBet;

	Player->CurrentChips -= CallAmount;
	Player->CurrentBet = CurrentMaxBet;
	Player->bHasActedThisRound = true;

	UE_LOG(LogTemp, Warning, TEXT("[Betting Log] %s called %d (Total bet : %d)"),
		*Player->GetPlayerName(), CallAmount, Player->CurrentBet);
}

void AHoldemGameState::ProcessRaise(AHoldemPlayerState* Player)
{
	if (!HasAuthority() || !Player) return;

	// Cap 시 해제
	// if (CurrentRaiseCount >= MaxRaise)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("[ProcessRaise] Max raises reached!"));
	// 	return;
	// }

	// Fixed Limit : 현재 Phase에 따른 베팅 단위
	// PreFlop/Flop -> 5 , Turn/River -> 10
	int32 BettingUnit = (CurrentPhase == EHoldemPhase::Turn ||
						CurrentPhase == EHoldemPhase::River)
						? BigBetAmount : SmallBetAmount;

	// Raise 후 새로운 MaxBet 계산
	int32 NewMaxBet = CurrentMaxBet + BettingUnit;
	// 내가 추가로 내야할 금액
	int32 AdditionalAmount = NewMaxBet - Player->CurrentBet;

	Player->CurrentChips -= AdditionalAmount;
	Player->CurrentBet = NewMaxBet;
	Player->bHasActedThisRound = true;

	// 새로운 최고 베팅액 설정
	CurrentMaxBet = NewMaxBet;

	// 한 턴 더 돌기
	for (APlayerState* PS : PlayerArray)
	{
		AHoldemPlayerState* HPS = Cast<AHoldemPlayerState>(PS);
		if (HPS && HPS != Player && !HPS->bIsFolded)
		{
			HPS->bHasActedThisRound = false;
		}
	}

	// Cap 시 해제
	//CurrentRaiseCount++;

	UE_LOG(LogTemp, Warning, TEXT("[Betting Log] %s raised by %d (Total bet: %d, Max bet: %d)"),
				*Player->GetPlayerName(), AdditionalAmount, Player->CurrentBet, CurrentMaxBet);
}

void AHoldemGameState::CollectBetsIntoPot()
{
	if (!HasAuthority()) return;

	for (APlayerState* PS : PlayerArray)
	{
		AHoldemPlayerState* Player = Cast<AHoldemPlayerState>(PS);
		if (Player)
		{
			// 플레이어별 이번 판 베팅액을 팟에 합산
			TotalPot += Player->CurrentBet;
			// 플레이어별 TotalBet에 누적
			Player->TotalBet += Player->CurrentBet;
			// CurrentBet 리셋
			Player->CurrentBet = 0;
			// 플래그 리셋
			Player->bHasActedThisRound = false;
		}
	}

	CurrentMaxBet = 0;

	UE_LOG(LogTemp, Warning, TEXT("[CollectBetsIntoPot] Current TotalPot : %d"), TotalPot);
}

int32 AHoldemGameState::GetSmallBlindIndex() const
{
	int32 PlayerCount = PlayerArray.Num();
	if (PlayerCount == 0) return -1;

	// Heads-Up (2인)
	// P1 - Dealer/Small Blind, P2 - Big Blind
	if (PlayerCount == 2)
	{
		return DealerPosition;
	}

	// 3인 이상
	return (DealerPosition + 1) % PlayerCount;
}

int32 AHoldemGameState::GetBigBlindIndex() const
{
	int32 PlayerCount = PlayerArray.Num();
	if (PlayerCount == 0) return -1;

	// Heads-Up (2인)
	if (PlayerCount == 2)
	{
		return (DealerPosition + 1) % PlayerCount;
	}

	// 3인 이상
	return (DealerPosition +2) % PlayerCount;
}

int32 AHoldemGameState::GetFirstPlayerIndex() const
{
	int32 PlayerCount = PlayerArray.Num();
	if (PlayerCount == 0) return -1;

	// PreFlop : Big Blind 다음부터
	if (CurrentPhase == EHoldemPhase::PreFlop)
	{
		int32 BBIndex = GetBigBlindIndex();
		return (BBIndex + 1) % PlayerCount;
	}

	// Flop/Turn/River : Small Blind부터 시작
	// (Heads-Up에서는 Big Blind부터)
	if (PlayerCount == 2)
	{
		return GetBigBlindIndex();
	}
	else
	{
		return GetSmallBlindIndex();
	}
}

void AHoldemGameState::RotateDealer()
{
	if (!HasAuthority()) return;

	int32 PlayerCount = PlayerArray.Num();
	if (PlayerCount == 0) return;

	// Dealer 포지션 시계방향으로 회전 (다음 플레이어)
	DealerPosition = (DealerPosition + 1) % PlayerCount;

	UE_LOG(LogTemp, Warning, TEXT("[RotateDealer] New Dealer position: %d"),
		DealerPosition);
}

void AHoldemGameState::SpawnPlayerItem()
{
	if (!HasAuthority()) return;

	for (APlayerState* PlayerState : PlayerArray)
	{
		AHoldemPlayerState* PS = Cast<AHoldemPlayerState>(PlayerState);
		if (PS)
		{
			// None 타입이면 스킵
			if (PS->SelectedItem == EItemType::None) continue;

			APawn* Player = PS->GetPawn();
			FVector PlayerLocation = Player->GetActorLocation();
			
			FVector ForwardDirection = GetForwardDirection(Player);
			FVector RightDirection = FVector::CrossProduct(FVector::UpVector, ForwardDirection);

			FVector BaseLocation = PlayerLocation + (ForwardDirection * ItemSpawnDistance);
			BaseLocation.Z = TableHeight;
			BaseLocation += RightDirection * ItemDistanceFromHalf;
			
			AItem* NewItem = GetWorld()->SpawnActor<AItem>(
				ItemClass, BaseLocation, FRotator::ZeroRotator);

			if (NewItem)
			{
				NewItem->ItemType = PS->SelectedItem;
				NewItem->SetItemMesh();
			}
		}
	}
}
