// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/HoldemGameState.h"
#include "Net/UnrealNetwork.h"

AHoldemGameState::AHoldemGameState()
{
	MaxPlayers = 4;
}

void AHoldemGameState::BeginPlay()
{
	Super::BeginPlay();
}

void AHoldemGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHoldemGameState, SpawnedCards);
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

void AHoldemGameState::SpawnTest()
{
	if (!HasAuthority()) return;

	GenerateDeck();
	ShuffleDeck();

	FVector TableCenter = FVector(220.000000,-40.000000,60.000000);

	FCardData DrawCard = Deck.Pop();
	
	SpawnCard(DrawCard, TableCenter, FRotator::ZeroRotator);

	UE_LOG(LogTemp, Warning, TEXT("New Card : %s, %s"),
			*UEnum::GetValueAsString(DrawCard.Suit), *UEnum::GetValueAsString(DrawCard.Rank));
	
}

void AHoldemGameState::ResetAllCardsLocation()
{
	if (!HasAuthority()) return;

	//for (ACard)
}
