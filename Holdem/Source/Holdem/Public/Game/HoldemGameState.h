// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Card.h"
#include "HandEvaluator.h"
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

// 플레이어의 개별 최고패 저장
USTRUCT(BlueprintType)
struct FPlayerHandPair
{
	GENERATED_BODY()

	UPROPERTY()
	APlayerState* Player;

	UPROPERTY()
	FHandEvaluation Evaluate;

	// 기본 생성자                                                                                                                                                    
	FPlayerHandPair() : Player(nullptr) {}
	// 파라미터 생성자                                                                                                                                                
	FPlayerHandPair(APlayerState* InPlayer, FHandEvaluation InEval)
		: Player(InPlayer), Evaluate(InEval) {}
};

USTRUCT(BlueprintType)
struct FShowdownResult
{
	GENERATED_BODY()

	// 승자 이름들
	UPROPERTY(BlueprintReadOnly)
	FString WinnerNames;
	// 최고 HandRank
	UPROPERTY(BlueprintReadOnly)
	EHandRank HighestRank;

	FShowdownResult()
	: WinnerNames(TEXT("")), HighestRank(EHandRank::HighCard) {}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhaseChanged, EHoldemPhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurnChanged, int32, NewTurnIndex);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnShowdownResultChanged,
	FString, WinnerNames, EHandRank, HighestRank);

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
	int32 MaxPlayers = 4;

public:
	//---------------------------------------------------//
	// Deck Management
	//---------------------------------------------------//
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
	//---------------------------------------------------//
	// Game Phase
	//---------------------------------------------------//
	UPROPERTY(ReplicatedUsing=OnRep_CurrentPhase, BlueprintReadOnly, Category = "GamePhase")
	EHoldemPhase CurrentPhase;

	EHoldemPhase PreviousPhase;

	// Look 입력 비활성화 (Phase 전환 연출용)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Input")
	bool bIsLookDisabled;

	UFUNCTION()
	void OnRep_CurrentPhase();

	UPROPERTY(BlueprintAssignable, Category = "GamePhase")
	FOnPhaseChanged OnPhaseChanged;

	// Waiting - 타이머
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "GamePhase")
	float WaitingTimeRemaining;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GamePhase")
	float WaitingDuration = 10.f;

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
	//---------------------------------------------------//
	// Player Cards
	//---------------------------------------------------//
	// 플레이어 앞 배치 거리
	UPROPERTY(EditDefaultsOnly, Category = "PlayerCards")
	float PlayerCardSpawnDistance = 85.f;
	// 두 카드 간 간격
	UPROPERTY(EditDefaultsOnly, Category = "PlayerCards")
	float PlayerCardSpacing = 15.f;

protected:
	// 플레이어 카드 배치 위치
	void GetPlayerCardSpawnLocation(APlayerState* PlayerState,
		FVector& OutFirstCardLoc, FVector& OutSecondCardLoc);

	// 플레이어가 테이블 보는 방향
	FVector GetForwardDirection(AActor* Player);
	
public:
	//---------------------------------------------------//
	// Community Cards
	//---------------------------------------------------//
	// 공개 카드
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "CommunityCards")
	TArray<ACard*> CommunityCards;
	
	// 테이블 중앙
	UPROPERTY(EditDefaultsOnly, Category = "CommunityCards")
	FVector TableLocation =
		FVector(0,0,0);

	// 테이블 높이
	UPROPERTY(EditDefaultsOnly, Category = "CommunityCards")
	float TableHeight = 90.f;

	// Community Cards 배열 회전 각도 (도 단위)
	UPROPERTY(EditDefaultsOnly, Category = "CommunityCards")
	float CommunityCardRotationAngle = 45.f;

	// 두 카드 간 간격
	UPROPERTY(EditDefaultsOnly, Category = "CommunityCards")
	float CommunityCardSpacing = 19.3f;

protected:
	// 공개 카드 테이블에 배치
	void SpawnCommunityCard(int32 CardIdx, float RotationAngle);

public:
	//---------------------------------------------------//
	// Showdown
	//---------------------------------------------------//
	UPROPERTY()
	class UHandEvaluator* HandEvaluator;

	// 승자 판정 (동점 승자 모두 저장)
	UFUNCTION(BlueprintCallable, Category = "GamePhase")
	TArray<APlayerState*> DetermineWinner(const TArray<APlayerState*>& ActivePlayers);

protected:
	// 결과 UI Replicate 정보
	UPROPERTY(ReplicatedUsing=OnRep_ShowdownResult, BlueprintReadOnly, Category="GamePhase")
	FShowdownResult CurrentShowdownResult;

	UFUNCTION()
	void OnRep_ShowdownResult();

public:
	UPROPERTY(BlueprintAssignable, Category = "GamePhase")
	FOnShowdownResultChanged OnShowdownResultChanged;

	// ResultWidget 결과 설정
	void SetShowdownResult(const FString& WinnerNames, EHandRank HighstRank);
	// ResultWidget 결과 초기화
	void ClearShowdownResult();
	
public:
	//---------------------------------------------------//
	// Betting
	//---------------------------------------------------//
	// 총 팟 금액
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Betting")
	int32 TotalPot = 0;
	// 최고 베팅액
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Betting")
	int32 CurrentMaxBet = 0;
	
	// 블라인드 설정 (Fixed Limit 5/10)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Betting")
	int32 SmallBlindAmount = 2;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Betting")
	int32 BigBlindAmount = 5;

	// PreFlop, Flop call amount
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Betting")
	int32 SmallBetAmount = 5;
	// Turn, River call amount
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Betting")
	int32 BigBetAmount = 10;

	// 테이블 입장/유지 최소 칩
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Betting")
	int32 MinimumChips = 100;

	// 현재 베팅 라운드의 레이즈 횟수
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Betting")
	int32 CurrentRaiseCount = 0;

	// 최대 레이즈 횟수 (1 Bet + 3 Raise)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Betting")
	int32 MaxRaise = 4;
	
public:
	// 현재 턴 플레이어 인덱스 (-1 : 베팅중 아님)
	UPROPERTY(ReplicatedUsing = OnRep_CurrentTurnPlayerIndex, BlueprintReadOnly, Category = "Betting")
	int32 CurrentTurnPlayerIndex = -1;
	
	UFUNCTION()
	void OnRep_CurrentTurnPlayerIndex();

	// 베팅 플레이어 전환 시 호출되는 deligate
	UPROPERTY(BlueprintAssignable, Category = "Betting")
	FOnTurnChanged OnTurnChanged;
	
	// 베팅 액션
	void ProcessFold(class AHoldemPlayerState* Player);
	void ProcessCheck(class AHoldemPlayerState* Player);
	void ProcessCall(class AHoldemPlayerState* Player);
	void ProcessRaise(class AHoldemPlayerState* Player);

	// 베팅 라운드 종료 시 팟에 합산
	void CollectBetsIntoPot();

public:
	//---------------------------------------------------//
	// Dealer Position
	//---------------------------------------------------//
	// Dealer Position
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Betting")
	int32 DealerPosition = 0;

	UFUNCTION(BlueprintPure, Category = "Betting")
	int32 GetSmallBlindIndex() const;

	UFUNCTION(BlueprintPure, Category = "Betting")
	int32 GetBigBlindIndex() const;
	// 베팅 시작할 플레이어 인덱스
	UFUNCTION(BlueprintPure, Category = "Betting")
	int32 GetFirstPlayerIndex() const;

	// Dealer 포지션 설정
	UFUNCTION(BlueprintCallable, Category = "Betting")
	void RotateDealer();
	// Position UI 업데이트
	void AssignPositions();
	
public:
	//---------------------------------------------------//
	// Item
	//---------------------------------------------------//
	UPROPERTY(EditDefaultsOnly, Category = "Item")
	TSubclassOf<class AItem> ItemClass;

	// 아이템 앞 배치 거리
	UPROPERTY(EditDefaultsOnly, Category = "Item")
	float ItemSpawnDistance = 65.f;
	// 아이템 옆 배치 거리
	UPROPERTY(EditDefaultsOnly, Category = "Item")
	float ItemDistanceFromHalf = 30.f;
	
	// 플레이어 아이템 배치
	void SpawnPlayerItem();

public:
	//---------------------------------------------------//
	// Utils
	//---------------------------------------------------//
	// 모든 카드 원위치
	UFUNCTION(BlueprintCallable, Category = "Deck")
	void ResetAllCardsLocation();
};
