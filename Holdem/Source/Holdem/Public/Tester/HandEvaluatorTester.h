// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Card.h"
#include "Game/HandEvaluator.h"
#include "HandEvaluatorTester.generated.h"

USTRUCT(BlueprintType)
struct FPlayerTestData
{
	GENERATED_BODY()
	
	// 식별용 이름
	UPROPERTY(EditAnywhere, Category = "Player")
	FString PlayerName;

	// 플레이어의 손패 2장
	UPROPERTY(EditAnywhere, Category = "Player")
	TArray<FCardData> HandCards;

	// 기본 생성자
	FPlayerTestData() : PlayerName(TEXT("Player")) {}
};

UCLASS()
class HOLDEM_API AHandEvaluatorTester : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHandEvaluatorTester();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	//---------------------------------------------------//
	// Test 1: 7장 입력 -> 가장 높은 HandRank반환
	//---------------------------------------------------//

	// [입력] 카드 7장
	UPROPERTY(EditAnywhere, Category = "Test1")
	TArray<FCardData> Cards;

	// [출력] 평가 결과
	UPROPERTY(VisibleAnywhere, Category = "Test1")
	FString Test1Result;

	// [실행 버튼]
	UFUNCTION(CallInEditor, Category = "Test1")
	void EvaluateHand();

public:
	//---------------------------------------------------//
	// Test 2: 여러 플레이어 중 승자 결정 (GameState의 DetermineWinner 테스트)
	//---------------------------------------------------//

	// [입력] 공개 카드 5장 (모든 플레이어 공통)
	UPROPERTY(EditAnywhere, Category = "Test2")
	TArray<FCardData> CommunityCards;

	// [입력] 플레이어들 데이터 (각 플레이어의 손패 2장)
	UPROPERTY(EditAnywhere, Category = "Test2")
	TArray<FPlayerTestData> PlayerTestData;

	// [출력] 승자 판정 결과
	UPROPERTY(VisibleAnywhere, Category = "Test2")
	FString Test2Result;

	// [실행 버튼]
	UFUNCTION(CallInEditor, Category = "Test2")
	void DetermineTestWinner();

private:
	UPROPERTY()
	class UHandEvaluator* HandEvaluator;

private:
	// 헬퍼 함수: FHandEvaluation을 읽기 쉬운 문자열로 변환
	FString HandEvaluationToString(const FHandEvaluation& Eval);

	// 헬퍼 함수: EHandRank를 문자열로 변환
	FString HandRankToString(EHandRank Rank);
};
