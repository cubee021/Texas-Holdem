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

UCLASS()
class HOLDEM_API AHoldemPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AHoldemPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
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

public:
	//---------------------------------------------------//
	// Game State
	//---------------------------------------------------//
	// 폴드 여부
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
	bool bIsFolded = false;

	// 플레이어 포지션(베팅 때 구현)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
	EPlayerPosition Position;

public:
	//---------------------------------------------------//
	// Chips (베팅때 구현 예정)
	//---------------------------------------------------//
	// 현재 칩 개수
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Chips")
	int32 CurrentChips;

	// 올인 상태 체크
	FORCEINLINE bool IsAllIn() const {return CurrentChips == 0 && !bIsFolded;}
};
