// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/HoldemGameState.h"

#include "Game/HoldemPlayerState.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

AHoldemGameState::AHoldemGameState()
{
	MaxPlayers = 4;
	CurrentPhase = EHoldemPhase::Waiting;
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
/*
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
*/
void AHoldemGameState::ResetAllCardsLocation()
{
	if (!HasAuthority()) return;

	//for (ACard)
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
			GetPlayerCardSpawnLocation(PS,FirstCardLoc, SecondCardLoc);

			// 플레이어가 보기에 카드가 세로로 놓이도록
			APawn* Player = PS->GetPawn();
			FRotator CardRotation = FRotationMatrix::MakeFromYZ(
				Player->GetActorForwardVector(), FVector::UpVector).Rotator();

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
		SpawnCommunityCard(i);
	}
}

void AHoldemGameState::DealTurnCard()
{
	if (!HasAuthority()) return;

	SpawnCommunityCard(3);
}

void AHoldemGameState::DealRiverCard()
{
	if (!HasAuthority()) return;

	SpawnCommunityCard(4);
}

void AHoldemGameState::GetPlayerCardSpawnLocation(APlayerState* PlayerState, FVector& OutFirstCardLoc,
                                                  FVector& OutSecondCardLoc)
{
	if (!PlayerState) return;

	APawn* Player = PlayerState->GetPawn();
	FVector PlayerLocation = Player->GetActorLocation();

	FVector BaseLocation = PlayerLocation +
		(Player->GetActorForwardVector() * SpawnDistanceFromPlayer);
	BaseLocation.Z = TableHeight;

	// BaseLocation 기준으로 좌우에 나란히 배치
	float HalfSpacing = CardSpacing / 2.f;
	
	OutFirstCardLoc = BaseLocation - (Player->GetActorRightVector() * HalfSpacing);
	OutSecondCardLoc = BaseLocation + (Player->GetActorRightVector() * HalfSpacing);
}

void AHoldemGameState::SpawnCommunityCard(int32 CardIdx)
{
	float Offset = (CardIdx - 2) * CardSpacing;
	FVector SpawnLocation = TableLocation + FVector(Offset, 0, 0);
	
	ACard* NewCard = DrawAndSpawnCard(SpawnLocation, FRotator::ZeroRotator);
	if (NewCard)
	{
		CommunityCards.Add(NewCard);
		NewCard->CardState = ECardState::OnTable;
	}
}
