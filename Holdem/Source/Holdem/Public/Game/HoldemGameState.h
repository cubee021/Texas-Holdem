// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Card.h"
#include "HoldemGameState.generated.h"

/**
 * 
 */

// Game Phase
UENUM(BlueprintType)
enum class EHoldemPhase : uint8
{
	Waiting		UMETA(DisplayName = "Waiting"), // 게임 시작 대기
	PreFlop		UMETA(DisplayName = "Preflop"), // 플레이어 2장 배분 + 첫 베팅
	Flop		UMETA(DisplayName = "Flop"),    // 테이블 3장 오픈 + 베팅
	Turn		UMETA(DisplayName = "Turn"),    // 테이블 4번째 카드 오픈 + 베팅
	River		UMETA(DisplayName = "River"),   // 테이블 5번째 카드 오픈 + 베팅
	Showdown	UMETA(DisplayName = "Showdown") // 승자 결정
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhaseChanged, EHoldemPhase, NewPhase);

UCLASS()
class HOLDEM_API AHoldemGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AHoldemGameState();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
public:
	// 현재 접속한 플레이어 수
	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetPlayerCount() const {return PlayerArray.Num();}

	// 최대 플레이어 수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxPlayers = 6;

public:
	// Deck Management
	// 덱 (52장 카드 데이터)
	UPROPERTY(BlueprintReadOnly, Category = "Deck")
	TArray<FCardData> Deck;
	// 맵에 스폰된 카드 액터들
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Deck")
	TArray<ACard*> SpawnedCards;
	// 카드 블루프린트 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Deck")
	TSubclassOf<ACard> CardClass;

	// 덱 생성
	UFUNCTION(BlueprintCallable, Category = "Deck")
	void GenerateDeck();
	// 덱 셔플
	UFUNCTION(BlueprintCallable, Category = "Deck")
	void ShuffleDeck();
	// 덱에서 카드 뽑아서 스폰
	UFUNCTION(BlueprintCallable, Category = "Deck")
	ACard* DrawAndSpawnCard(FVector Location, FRotator Rotation);

protected:
	// 카드 맵에 스폰 (데이터 -> 액터)
	ACard* SpawnCard(const FCardData& Data, FVector Location, FRotator Rotation);

public:
	// 모든 카드 원위치
	UFUNCTION(BlueprintCallable, Category = "Deck")
	void ResetAllCardsLocation();

public:
	// Game Phase
	UPROPERTY(ReplicatedUsing=OnRep_CurrentPhase, BlueprintReadOnly, Category = "GamePhase")
	EHoldemPhase CurrentPhase;

	EHoldemPhase PreviousPhase;

	UFUNCTION()
	void OnRep_CurrentPhase();

	UPROPERTY(BlueprintAssignable, Category = "GamePhase")
	FOnPhaseChanged OnPhaseChanged;

	// Waiting - 타이머
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "GamePhase")
	float WaitingTimeRemaining;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GamePhase")
	float WaitingDuration = 15.f;

	// PreFlop - 플레이어당 2장씩 카드 배분
	UFUNCTION(BlueprintCallable, Category = "GamePhase")
	void DealPreflopToPlayers();
	// Flop - 테이블 중앙에 3장 배분
	UFUNCTION(BlueprintCallable, Category = "GamePhase")
	void DealFlopCards();
	// Turn - 테이블 중앙에 4번째 카드 배분
	UFUNCTION(BlueprintCallable, Category = "GamePhase")
	void DealTurnCard();
	// River - 테이블 중앙에 5번째 카드 배분
	UFUNCTION(BlueprintCallable, Category = "GamePhase")
	void DealRiverCard();
	
public:
	// Player Cards
	// 플레이어 앞 배치 거리
	UPROPERTY(EditDefaultsOnly, Category = "PlayerCards")
	float SpawnDistanceFromPlayer = 90.f;
	// 두 카드 간 간격
	UPROPERTY(EditDefaultsOnly, Category = "PlayerCards")
	float CardSpacing = 25.f;
	// 테이블 높이
	UPROPERTY(EditDefaultsOnly, Category = "PlayerCards")
	float TableHeight = 90.f;

protected:
	// 플레이어 카드 배치 위치
	void GetPlayerCardSpawnLocation(APlayerState* PlayerState,
		FVector& OutFirstCardLoc, FVector& OutSecondCardLoc);

public:
	// Community Cards
	// 공개 카드
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "CommunityCards")
	TArray<ACard*> CommunityCards;
	
	// 테이블 중앙
	UPROPERTY(EditDefaultsOnly, Category = "CommunityCards")
	FVector TableLocation =
		FVector(250.000000,20.000000,90.000000);
	
protected:
	// 공개 카드 테이블에 배치
	void SpawnCommunityCard(int32 CardIdx);

public:
	// Item
	UPROPERTY(EditDefaultsOnly, Category = "Item")
	TSubclassOf<class AItem> ItemClass;
	
	// 플레이어 아이템 배치
	void SpawnPlayerItem();
};
