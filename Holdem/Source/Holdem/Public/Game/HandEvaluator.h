// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Card.h"
#include "HandEvaluator.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EHandRank : uint8
{
	HighCard = 0		UMETA(DisplayName = "HighCard"),
	OnePair = 1			UMETA(DisplayName = "OnePair"),
	TwoPair = 2			UMETA(DisplayName = "TwoPair"),
	ThreeOfAKind = 3	UMETA(DisplayName = "ThreeOfAKind"),
	Straight = 4		UMETA(DisplayName = "Straight"),
	Flush = 5			UMETA(DisplayName = "Flush"),
	FullHouse = 6		UMETA(DisplayName = "FullHouse"),
	FourOfAKind = 7		UMETA(DisplayName = "FourOfAKind"),
	StraightFlush = 8	UMETA(DisplayName = "StraightFlush"),
	RoyalFlush = 9		UMETA(DisplayName = "RoyalFlush")
};

USTRUCT(BlueprintType)
struct FHandEvaluation
{
	GENERATED_BODY()
	// 랭크
	UPROPERTY(BlueprintReadOnly)
	EHandRank Rank;

	// 핸드를 구성하는 카드들의 숫자 (내림차순 정렬)
	UPROPERTY(BlueprintReadOnly)
	TArray<int32> CardValues;

	// 기본 생성자
	FHandEvaluation() : Rank(EHandRank::HighCard) {}

	// 비교 연산자
	bool operator>(const FHandEvaluation& Other) const
	{
		// 1. Rank 비교 (숫자가 클수록 강함)
		if (Rank != Other.Rank)
		{
			return (uint8)Rank > (uint8)Other.Rank;
		}

		// 2. Rank가 같으면 CardValues 순서대로 비교
		int32 MinSize = FMath::Min(CardValues.Num(), Other.CardValues.Num());
		for (int32 i = 0; i < MinSize; i++)
		{
			if (CardValues[i] != Other.CardValues[i])
			{
				return CardValues[i] > Other.CardValues[i];
			}
		}

		// 3. 모두 같으면 동점
		return false;
	}

};

UCLASS()
class HOLDEM_API UHandEvaluator : public UObject
{
	GENERATED_BODY()
	
public:
	// 5장에서 최고패 판정
	FHandEvaluation EvaluateHand(TArray<ACard*> AllCards);

	// 7장 중 최고의 5장 조합 찾기
	FHandEvaluation GetBestHandFrom7Cards(const TArray<ACard*>& SevenCards);

private:
	// 조합 생성 재귀 헬퍼
	void GenerateCombinations(
		const TArray<ACard*>& AllCards,
		TArray<int32>& CurrentIndices,
		int32 Start,
		int32 Remaining,
		FHandEvaluation& BestHand);

private:
	// 카드 숫자 변환 (예외 : Ace = 14)
	int32 GetCardValue(ECardRank Rank);

	// Flush 체크
	bool IsFlush(const TArray<ACard*>& Cards,
		ECardSuit& OutSuit);
	// Straight 체크
	bool IsStraight(const TArray<int32>& CardValues,
		TArray<int32>& OutStraightCards);
	// 각 카드 숫자(Value)의 등장 횟수 체크
	void CountRanks(const TArray<int32>& CardValues,
		TMap<int32, int32>& OutCounts);
};
