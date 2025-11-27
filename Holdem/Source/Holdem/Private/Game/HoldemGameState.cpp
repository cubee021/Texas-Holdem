// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/HoldemGameState.h"

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
}

void AHoldemGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHoldemGameState, SpawnedCards);
	DOREPLIFETIME(AHoldemGameState, CurrentPhase);
	DOREPLIFETIME(AHoldemGameState, CommunityCards);
	DOREPLIFETIME(AHoldemGameState, WaitingTimeRemaining);
	DOREPLIFETIME(AHoldemGameState, bIsLookDisabled);
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
	// 여기 이렇게 구현한 이유?
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
			FVector PlayerLocation = Player->GetActorLocation();
			FVector ForwardDirection = (PlayerLocation - TableLocation).GetSafeNormal2D();

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
	FVector PlayerLocation = Player->GetActorLocation();

	// ★ 핵심: 테이블 중심 → 플레이어 방향 = 플레이어의 "앞"
	// Z축 무시하고 평면상 방향만 계산
	FVector ForwardDirection = (TableLocation-PlayerLocation).GetSafeNormal2D();

	FVector BaseLocation = PlayerLocation + (ForwardDirection * PlayerCardSpawnDistance);

	// ★ Forward와 직교하는 Right 벡터: Z축과 Forward의 외적
	FVector RightDirection = FVector::CrossProduct(FVector::UpVector, ForwardDirection);

	// BaseLocation 기준으로 좌우에 나란히 배치
	float HalfSpacing = PlayerCardSpacing / 2.f;

	OutFirstCardLoc = BaseLocation - (RightDirection * HalfSpacing);
	OutSecondCardLoc = BaseLocation + (RightDirection * HalfSpacing);
}

FVector AHoldemGameState::GetBaseLocationFromPlayer(APawn* Player, float Distance)
{
	FVector PlayerLocation = Player->GetActorLocation();

	FVector BaseLocation = PlayerLocation +
		(Player->GetActorForwardVector() * Distance);
	BaseLocation.Z = TableHeight;

	return BaseLocation;
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
	}
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
			FVector BaseLocation = GetBaseLocationFromPlayer(Player, ItemSpawnDistance);

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
