// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Item.h"
#include "HoldemPlayerState.generated.h"

/**
 * 
 */

// 플레이어 포지션
UENUM(BlueprintType)
enum class EPlayerPosition : uint8
{
	None		UMETA(DisplayName = "None"),
	Dealer		UMETA(DisplayName = "Dealer"),
	SmallBlind	UMETA(DisplayName = "SmallBlind"),
	BigBlind	UMETA(DisplayName = "BigBlind")
};

// 누른 베팅 버튼 정보
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBettingActionChanged, FString, BettingAction);
// 칩 & 베팅액
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBettingInfoChanged, int32, NewCurrentBet, int32, NewTotalBet);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCurrentChipsChanged, int32, NewChips);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPositionChanged, EPlayerPosition, NewPosition);
// 상태
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpectatingChanged, bool, bNewIsSpectating);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFoldChanged, bool, bNewIsFolded);
// 스팀 정보
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSteamInfoChanged, FString, SteamID, FString, SteamName);

UCLASS()
class HOLDEM_API AHoldemPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AHoldemPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

public:
	//---------------------------------------------------//
	// Player Info
	//---------------------------------------------------//

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
	int32 SeatIndex = -1;
	
public:
	//---------------------------------------------------//
	// Player Cards
	//---------------------------------------------------//
	// 플레이어가 소유한 카드
	UPROPERTY(BlueprintReadOnly, Category = "Hand")
	TArray<class ACard*> HandCards;

	// 카드 추가
	UFUNCTION(BlueprintCallable, Category = "Hand")
	void AddCard(class ACard* Card);
	// 패 초기화
	UFUNCTION(BlueprintCallable, Category = "Hand")
	void ClearHand();

public:
	//---------------------------------------------------//
	// Item
	//---------------------------------------------------//
	// Waiting Phase에서 선택한 아이템
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Item")
	EItemType SelectedItem;

	UFUNCTION(Server, Reliable)
	void Server_SetSelectedItem(EItemType NewItem);

protected:
	//---------------------------------------------------//
	// Steam Info
	//---------------------------------------------------//
	// Steam ID
	UPROPERTY(ReplicatedUsing=OnRep_SteamID, BlueprintReadOnly, Category = Steam)
	FString SteamID;

	UFUNCTION()
	void OnRep_SteamID();

	// Steam 닉네임
	UPROPERTY(ReplicatedUsing=OnRep_SteamName, BlueprintReadOnly, Category = Steam)
	FString SteamName;

	UFUNCTION()
	void OnRep_SteamName();

public:
	// Getters & Setters
	void SetSteamID(const FString& NewSteamID);
	void SetSteamName(const FString& NewSteamName);
	FORCEINLINE FString GetSteamID() const {return SteamID;}
	FORCEINLINE FString GetSteamName() const {return SteamName;}
	
protected:
	//---------------------------------------------------//
	// Game State
	//---------------------------------------------------//
	// 관전 모드 여부
	UPROPERTY(ReplicatedUsing=OnRep_IsSpectating, BlueprintReadOnly, Category = "Game")
	bool bIsSpectating = false;

	UFUNCTION()
	void OnRep_IsSpectating();

	// 폴드 여부
	UPROPERTY(ReplicatedUsing=OnRep_bIsFolded, BlueprintReadOnly, Category = "Game")
	bool bIsFolded = false;

	UFUNCTION()
	void OnRep_bIsFolded();
	
	// 플레이어 포지션
	UPROPERTY(ReplicatedUsing=OnRep_Position, BlueprintReadOnly, Category = "Game")
	EPlayerPosition Position;

	UFUNCTION()
	void OnRep_Position();
	
public:
	// Getters & Setters
	void SetIsSpectating(bool bNewIsSpectating);
	void SetIsFolded(bool bNewFolded);
	void SetPosition(EPlayerPosition NewPosition);

	FORCEINLINE bool GetIsSpectating() const {return bIsSpectating;}
	FORCEINLINE bool GetIsFolded() const {return bIsFolded;}
	FORCEINLINE EPlayerPosition GetPosition() const {return Position;}
	
protected:
	//---------------------------------------------------//
	// Chips
	//---------------------------------------------------//
	// 현재 칩 개수
	UPROPERTY(ReplicatedUsing = OnRep_CurrentChips, BlueprintReadOnly, Category = "Chips")
	int32 CurrentChips;

public:
	UFUNCTION()
	void OnRep_CurrentChips();

	// Getter & Setter
	void SetCurrentChips(int32 NewChips);
	FORCEINLINE int32 GetCurrentChips() const {return CurrentChips;}

	// 올인 상태 체크
	FORCEINLINE bool IsAllIn() const {return CurrentChips == 0 && !bIsFolded;}

protected:
	//---------------------------------------------------//
	// Betting
	//---------------------------------------------------//
	// 마지막 베팅 액션 (Raise, Bet, Call, Check, Fold)
	UPROPERTY(ReplicatedUsing = OnRep_LastBettingAction, BlueprintReadOnly, Category = "Betting")
	FString LastBettingAction;

	UFUNCTION()
	void OnRep_LastBettingAction();
	
	// Bet - 현재 라운드에 낼 금액
	UPROPERTY(ReplicatedUsing = OnRep_CurrentBet, BlueprintReadOnly, Category = "Betting")
	int32 CurrentBet = 0;

	UFUNCTION()
	void OnRep_CurrentBet();
	
	// Total Bet - 이번 판에 베팅한 총액
	UPROPERTY(ReplicatedUsing = OnRep_TotalBet, BlueprintReadOnly, Category = "Betting")
	int32 TotalBet = 0;

	UFUNCTION()
	void OnRep_TotalBet();

public:
	// Getters & Setters
	void SetLastBettingAction(const FString& Action);
	void SetCurrentBet(int32 NewBet);
	void SetTotalBet(int32 NewTotal);
	FORCEINLINE FString GetLastBettingAction() const {return LastBettingAction;}
	FORCEINLINE int32 GetCurrentBet() const {return CurrentBet;}
	FORCEINLINE int32 GetTotalBet() const {return TotalBet;}

	// 이번 베팅에 액션했는지 (flag)
	UPROPERTY(BlueprintReadOnly, Category = "Betting")
	bool bHasActedThisRound = false;

public:
	//---------------------------------------------------//
	// Deligates
	//---------------------------------------------------//
	UPROPERTY(BlueprintAssignable, Category = "Betting")
	FOnBettingActionChanged OnBettingActionChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnBettingInfoChanged OnBettingInfoChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCurrentChipsChanged OnCurrentChipsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPositionChanged OnPositionChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSpectatingChanged OnSpectatingChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnFoldChanged OnFoldChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSteamInfoChanged OnSteamInfoChanged;
};
