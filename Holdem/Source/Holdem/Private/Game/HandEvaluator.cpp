// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/HandEvaluator.h"

FHandEvaluation UHandEvaluator::EvaluateHand(TArray<ACard*> AllCards)
{
	FHandEvaluation Result;
	
	// 카드의 A~K값으로만 이루어진 배열 
	TArray<int32> CardValues;
	for (ACard* Card : AllCards)
	{
		CardValues.Add(GetCardValue(Card->CardData.Rank));
	}

	// 1. Flush 체크
	ECardSuit FlushSuit;
	bool bHasFlush = IsFlush(AllCards, FlushSuit);

	// 2. Straight 체크
	TArray<int32> StraightCards;
	bool bHasStraight = IsStraight(CardValues, StraightCards);

	// 3. Straight Flush 체크
	// (Flush로 무늬 걸러낸다 -> Straight로 연속된 숫자 체크한다)
	if (bHasFlush)
	{
		// Flush 무늬의 카드만 추출
		TArray<int32> FlushCardValues;
		for (ACard* Card : AllCards)
		{
			if (Card->CardData.Suit == FlushSuit)
			{
				FlushCardValues.Add(GetCardValue(Card->CardData.Rank));
			}
		}

		// Straight 체크
		TArray<int32> FlushStraightCards;
		if (IsStraight(FlushCardValues, FlushStraightCards))
		{
			// Ace-high
			if (FlushStraightCards[0] == 14)
			{
				// Royal Flush!
				Result.Rank = EHandRank::RoyalFlush;
				Result.CardValues = FlushStraightCards;
				return Result;
			}
			else
			{
				// Straight Flush!
				Result.Rank = EHandRank::StraightFlush;
				Result.CardValues = FlushStraightCards;
				return Result;
			}
		}
	}

	// 4. Rank 개수 체크
	TMap<int32, int32> RankCounts;
	CountRanks(CardValues, RankCounts);

	TArray<int32> Quads;	// 4장
	TArray<int32> Trips;	// 3장
	TArray<int32> Pairs;	// 2장
	TArray<int32> Singles;	// 1장

	for (auto& Pair : RankCounts)
	{
		int32 Value = Pair.Key;
		int32 Count = Pair.Value;

		if		(Count == 4) Quads.Add(Value);
		else if (Count == 3) Trips.Add(Value);
		else if (Count == 2) Pairs.Add(Value);
		else if (Count == 1) Singles.Add(Value);
	}

	// 내림차순 정렬
	Quads.Sort([](int32 A, int32 B) {return A>B;});
	Trips.Sort([](int32 A, int32 B) {return A>B;});
	Pairs.Sort([](int32 A, int32 B) {return A>B;});
	Singles.Sort([](int32 A, int32 B) {return A>B;});

	// Four of a Kind!
	if (Quads.Num() > 0)
	{
		Result.Rank = EHandRank::FourOfAKind;
		Result.CardValues.Add(Quads[0]);
		Result.CardValues.Add(Quads[0]);
		Result.CardValues.Add(Quads[0]);
		Result.CardValues.Add(Quads[0]);

		// 키커 1장
		if (Trips.Num() > 0) Result.CardValues.Add(Trips[0]);
		else if (Pairs.Num() > 0) Result.CardValues.Add(Pairs[0]);
		else Result.CardValues.Add(Singles[0]);

		return Result;
	}

	// Full House!
	if (Trips.Num() > 0 && (Pairs.Num() > 0 || Trips.Num() > 1))
	{
		Result.Rank = EHandRank::FullHouse;

		// 가장 높은 Triple
		Result.CardValues.Add(Trips[0]);
		Result.CardValues.Add(Trips[0]);
		Result.CardValues.Add(Trips[0]);
		
		if (Trips.Num() > 1)
		{
			// 트리플 중에 두 장
			Result.CardValues.Add(Trips[1]);
			Result.CardValues.Add(Trips[1]);
		}
		else
		{
			// 투페어 중에 두 장
			Result.CardValues.Add(Pairs[0]);
			Result.CardValues.Add(Pairs[0]);
		}

		return Result;
	}

	// Flush!
	if (bHasFlush)
	{
		Result.Rank = EHandRank::Flush;

		// Flush 무늬의 카드 5장 (높은 순)
		TArray<int32> FlushCardValues;
		for (ACard* Card : AllCards)
		{
			if (Card->CardData.Suit == FlushSuit)
			{
				FlushCardValues.Add(GetCardValue(Card->CardData.Rank));
			}
		}
		FlushCardValues.Sort([](int32 A, int32 B) { return A > B; });

		for (int32 i = 0; i < 5; i++)
		{
			Result.CardValues.Add(FlushCardValues[i]);
		}

		return Result;
	}

	// Straight!
	if (bHasStraight)
	{
		Result.Rank = EHandRank::Straight;
		Result.CardValues = StraightCards;
		
		return Result;
	}

	// Three of a kind!
	if (Trips.Num() > 0)
	{
		Result.Rank = EHandRank::ThreeOfAKind;
		Result.CardValues.Add(Trips[0]);
		Result.CardValues.Add(Trips[0]);
		Result.CardValues.Add(Trips[0]);

		// 키커 2장
		int32 KickerCount = 0;
		for (int32 single : Singles)
		{
			Result.CardValues.Add(single);
			if (++KickerCount >= 2) break;
		}

		return Result;
	}

	// Two Pair!
	if (Pairs.Num() >= 2)
	{
		Result.Rank = EHandRank::TwoPair;
		Result.CardValues.Add(Pairs[0]);
		Result.CardValues.Add(Pairs[0]);
		Result.CardValues.Add(Pairs[1]);
		Result.CardValues.Add(Pairs[1]);

		// 키커 1장
		if (Singles.Num() > 0)	  Result.CardValues.Add(Singles[0]);
		else if (Pairs.Num() > 2) Result.CardValues.Add(Pairs[2]);

		return Result;
	}

	// One Pair!
	if (Pairs.Num() == 1)
	{
		Result.Rank = EHandRank::OnePair;
		Result.CardValues.Add(Pairs[0]);
		Result.CardValues.Add(Pairs[0]);

		// 키커 3장
		int32 KickerCount = 0;
		for (int32 Single : Singles)
		{
			Result.CardValues.Add(Single);
			if (++KickerCount >= 3) break;
		}

		return Result;
	}

	// High Card!
	Result.Rank = EHandRank::HighCard;
	Singles.Sort([](int32 A, int32 B) { return A > B; });
	for (int32 i=0; i<5; i++)
	{
		Result.CardValues.Add(Singles[i]);
	}

	return Result;
}

int32 UHandEvaluator::GetCardValue(ECardRank Rank)
{
	if (Rank == ECardRank::Ace) return 14;

	return (int32)Rank + 1;
}

bool UHandEvaluator::IsFlush(const TArray<ACard*>& Cards, ECardSuit& OutSuit)
{
	// 무늬별 개수 count
	int32 SuitCounts[4] = {0};

	for (ACard* Card : Cards)
	{
		SuitCounts[(int32)Card->CardData.Suit]++;
	}

	// 5장 이상인 무늬 찾기
	for (int32 i=0; i<4; i++)
	{
		if (SuitCounts[i] >= 5)
		{
			// 공통된 무늬 결과로 세팅
			OutSuit = (ECardSuit)i;
			return true;
		}
	}

	return false;
}

bool UHandEvaluator::IsStraight(const TArray<int32>& CardValues, TArray<int32>& OutStraightCards)
{
	// 중복 X - Straight는 서로 다른 숫자로만 구성
	// 내림차순으로 배열 만들기
	TArray<int32> UniqueValues = CardValues;

	// 중복제거
	for (int32 i=UniqueValues.Num() - 1; i>= 0; i--)
	{
		for (int32 j=i-1; j>=0; j--)
		{
			if (UniqueValues[i] == UniqueValues[j])
			{
				UniqueValues.RemoveAt(i);
				break;
			}
		}
	}
	
	// 내림차순 정렬
	UniqueValues.Sort([](int32 A, int32 B){return A>B;});

	// 연속된 5장 찾기
	for (int32 i=0; i<=UniqueValues.Num() - 5; i++)
	{
		bool bIsStraight = true;

		for (int32 j=0; j<4; j++)
		{
			if (UniqueValues[i+j] - UniqueValues[i+j+1] != 1)
			{
				bIsStraight = false;
				break;
			}
		}

		if (bIsStraight)
		{
			OutStraightCards.Empty();
			for (int32 j=0; j<5; j++)
			{
				OutStraightCards.Add(UniqueValues[i+j]);
			}

			return true;
		}
	}

	// A-2-3-4-5 (Wheel) 체크
	if (UniqueValues.Contains(14) &&
	 UniqueValues.Contains(2) &&
	 UniqueValues.Contains(3) &&
	 UniqueValues.Contains(4) &&
	 UniqueValues.Contains(5))
	{
		// Wheel Straight (5-high)
		OutStraightCards.Empty();
		OutStraightCards.Add(5);
		OutStraightCards.Add(4);
		OutStraightCards.Add(3);
		OutStraightCards.Add(2);
		OutStraightCards.Add(1);  // Ace를 1로
			
		return true;
	}

	return false;
}

void UHandEvaluator::CountRanks(const TArray<int32>& CardValues, TMap<int32, int32>& OutCounts)
{
	OutCounts.Empty();

	for (int32 Value : CardValues)
	{
		OutCounts.FindOrAdd(Value)++;
	}
}

FHandEvaluation UHandEvaluator::GetBestHandFrom7Cards(const TArray<ACard*>& SevenCards)
{
	// 7장이 아니면 그냥 EvaluateHand 호출
	if (SevenCards.Num() != 7)
	{
		return EvaluateHand(SevenCards);
	}

	// 최고 핸드 초기화 (가장 낮은 핸드로 시작)
	FHandEvaluation BestHand;
	BestHand.Rank = EHandRank::HighCard;
	BestHand.CardValues.Empty();

	// 현재 선택한 인덱스 배열
	TArray<int32> CurrentIndices;
	CurrentIndices.Reserve(5);

	// 재귀로 모든 조합 생성 및 평가
	GenerateCombinations(SevenCards, CurrentIndices, 0, 5, BestHand);

	return BestHand;
}

void UHandEvaluator::GenerateCombinations(
	const TArray<ACard*>& AllCards,
	TArray<int32>& CurrentIndices,
	int32 Start,
	int32 Remaining,
	FHandEvaluation& BestHand)
{
	// Base case: 5개 선택 완료
	if (Remaining == 0)
	{
		// 선택한 인덱스로 카드 배열 만들기
		TArray<ACard*> FiveCards;
		FiveCards.Reserve(5);
		for (int32 Idx : CurrentIndices)
		{
			FiveCards.Add(AllCards[Idx]);
		}

		// 핸드 평가
		FHandEvaluation CurrentHand = EvaluateHand(FiveCards);

		// 최고 핸드 업데이트
		if (CurrentHand > BestHand)
		{
			BestHand = CurrentHand;
		}

		return;
	}

	// Recursive case: Start부터 순회하며 선택
	for (int32 i = Start; i <= AllCards.Num() - Remaining; i++)
	{
		// i번째 카드 선택
		CurrentIndices.Add(i);

		// 재귀 호출 (다음 선택: i+1부터, 남은 개수: Remaining-1)
		GenerateCombinations(AllCards, CurrentIndices, i + 1, Remaining - 1, BestHand);

		// 백트래킹 (선택 취소)
		CurrentIndices.Pop();
	}
}
