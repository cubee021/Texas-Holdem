// Fill out your copyright notice in the Description page of Project Settings.

#include "Tester/HandEvaluatorTester.h"
#include "Game/HandEvaluator.h"
#include "Game/HoldemGameState.h"
#include "Game/HoldemPlayerState.h"

// Sets default values
AHandEvaluatorTester::AHandEvaluatorTester()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AHandEvaluatorTester::BeginPlay()
{
	Super::BeginPlay();

	HandEvaluator = NewObject<UHandEvaluator>(this);
}

//---------------------------------------------------//
// Test 1: 7장 입력 -> 가장 높은 HandRank반환
//---------------------------------------------------//
void AHandEvaluatorTester::EvaluateHand()
{
	if (Cards.Num() < 7) return;
	
	if (!HandEvaluator)
	{
		HandEvaluator = NewObject<UHandEvaluator>(this);
	}

	TArray<ACard*> TempCards;
	for (const FCardData& CardData : Cards)
	{
		// 임시 카드 액터 스폰
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		ACard* TempCard = GetWorld()->SpawnActor<ACard>(ACard::StaticClass(),SpawnParams);

		if (TempCard)
		{
			TempCard->SetCardData(CardData);
			TempCards.Add(TempCard);
		}
	}

	FHandEvaluation Result = HandEvaluator->GetBestHandFrom7Cards(TempCards);

	Test1Result = HandEvaluationToString(Result);

	// 임시 액터 정리
	for (ACard* Card : TempCards)
		if (Card) Card->Destroy();
}

//---------------------------------------------------//
// Test 2: 여러 플레이어 중 승자 결정 (GameState의 DetermineWinner 테스트)
//---------------------------------------------------//
void AHandEvaluatorTester::DetermineTestWinner()
{
	// 1. 입력 검증
	if (PlayerTestData.Num() < 2)
	{
		Test2Result = FString::Printf(TEXT("Error: Need at least 2 players, got %d"), PlayerTestData.Num());
		UE_LOG(LogTemp, Error, TEXT("[HandEvaluatorTester] %s"), *Test2Result);
		return;
	}

	if (CommunityCards.Num() != 5)
	{
		Test2Result = FString::Printf(TEXT("Error: Need exactly 5 community cards, got %d"), CommunityCards.Num());
		UE_LOG(LogTemp, Error, TEXT("[HandEvaluatorTester] %s"), *Test2Result);
		return;
	}

	// 2. GameState 가져오기
	AHoldemGameState* GameState = Cast<AHoldemGameState>(GetWorld()->GetGameState());
	if (!GameState)
	{
		Test2Result = TEXT("Error: HoldemGameState not found in world!");
		UE_LOG(LogTemp, Error, TEXT("[HandEvaluatorTester] %s"), *Test2Result);
		return;
	}

	// 3. 공개 카드 임시 설정 (원본 백업)
	TArray<ACard*> OriginalCommunityCards = GameState->CommunityCards;
	GameState->CommunityCards.Empty();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	for (const FCardData& CardData : CommunityCards)
	{
		ACard* CommunityCard = GetWorld()->SpawnActor<ACard>(ACard::StaticClass(), SpawnParams);
		if (CommunityCard)
		{
			CommunityCard->SetCardData(CardData);
			GameState->CommunityCards.Add(CommunityCard);
		}
	}

	// 4. 가짜 PlayerState들 생성 및 HandCards 할당
	TArray<APlayerState*> TestPlayers;
	TArray<AHoldemPlayerState*> CreatedPlayerStates; // 정리용

	FString DetailedResult = TEXT("=== GameState DetermineWinner Test ===\n\n");
	DetailedResult += TEXT("Community Cards: ");
	for (int32 i = 0; i < GameState->CommunityCards.Num(); i++)
	{
		if (i > 0) DetailedResult += TEXT(", ");
		// CardData 정보 표시 로직 추가 가능
	}
	DetailedResult += TEXT("\n\n");

	for (const FPlayerTestData& PlayerData : PlayerTestData)
	{
		// 손패 검증
		if (PlayerData.HandCards.Num() != 2)
		{
			DetailedResult += FString::Printf(TEXT("[%s] Error: Need 2 hand cards, got %d\n\n"),
				*PlayerData.PlayerName, PlayerData.HandCards.Num());
			continue;
		}

		// 가짜 PlayerState 생성
		AHoldemPlayerState* TestPS = NewObject<AHoldemPlayerState>(this);
		if (!TestPS) continue;

		TestPS->SetPlayerName(PlayerData.PlayerName);
		TestPS->bIsFolded = false;

		// HandCards 할당
		for (const FCardData& CardData : PlayerData.HandCards)
		{
			ACard* HandCard = GetWorld()->SpawnActor<ACard>(ACard::StaticClass(), SpawnParams);
			if (HandCard)
			{
				HandCard->SetCardData(CardData);
				TestPS->HandCards.Add(HandCard);
			}
		}

		TestPlayers.Add(TestPS);
		CreatedPlayerStates.Add(TestPS);

		DetailedResult += FString::Printf(TEXT("[%s] Ready (Hand: %d cards)\n"),
			*PlayerData.PlayerName, TestPS->HandCards.Num());
	}

	DetailedResult += TEXT("\n=== Calling GameState->DetermineWinner() ===\n\n");

	// 5. GameState의 DetermineWinner() 직접 호출!
	TArray<APlayerState*> Winners = GameState->DetermineWinner(TestPlayers);

	// 6. 결과 출력
	if (Winners.Num() == 0)
	{
		DetailedResult += TEXT("No winners found!\n");
	}
	else if (Winners.Num() == 1)
	{
		DetailedResult += FString::Printf(TEXT("Winner: %s\n"), *Winners[0]->GetPlayerName());
	}
	else
	{
		DetailedResult += TEXT("Draw! Winners: ");
		for (int32 i = 0; i < Winners.Num(); i++)
		{
			DetailedResult += Winners[i]->GetPlayerName();
			if (i < Winners.Num() - 1)
			{
				DetailedResult += TEXT(", ");
			}
		}
		DetailedResult += TEXT("\n");
	}

	// 7. 정리
	// 생성한 카드 액터들 삭제
	for (ACard* Card : GameState->CommunityCards)
	{
		if (Card) Card->Destroy();
	}

	for (AHoldemPlayerState* PS : CreatedPlayerStates)
	{
		if (PS)
		{
			for (ACard* Card : PS->HandCards)
			{
				if (Card) Card->Destroy();
			}
		}
	}

	// GameState 원상 복구
	GameState->CommunityCards = OriginalCommunityCards;

	// 8. 결과 저장 및 로그 출력
	Test2Result = DetailedResult;
	UE_LOG(LogTemp, Warning, TEXT("[HandEvaluatorTester]\n%s"), *Test2Result);
}

FString AHandEvaluatorTester::HandEvaluationToString(const FHandEvaluation& Eval)
{
	FString Result = HandRankToString(Eval.Rank);

	// CardValues 추가                                                                                                                                              
	Result += TEXT(" [");
	for (int32 i = 0; i < Eval.CardValues.Num(); i++)
	{
		Result += FString::Printf(TEXT("%d"), Eval.CardValues[i]);
		if (i < Eval.CardValues.Num() - 1)
		{
			Result += TEXT(", ");
		}
	}
	Result += TEXT("]");

	return Result;
}

FString AHandEvaluatorTester::HandRankToString(EHandRank Rank)
{
	switch (Rank)
	{
	case EHandRank::HighCard:       return TEXT("High Card");
	case EHandRank::OnePair:        return TEXT("One Pair");
	case EHandRank::TwoPair:        return TEXT("Two Pair");
	case EHandRank::ThreeOfAKind:   return TEXT("Three of a Kind");
	case EHandRank::Straight:       return TEXT("Straight");
	case EHandRank::Flush:          return TEXT("Flush");
	case EHandRank::FullHouse:      return TEXT("Full House");
	case EHandRank::FourOfAKind:    return TEXT("Four of a Kind");
	case EHandRank::StraightFlush:  return TEXT("Straight Flush");
	case EHandRank::RoyalFlush:     return TEXT("Royal Flush");
	default:                        return TEXT("Unknown");
	}
}

