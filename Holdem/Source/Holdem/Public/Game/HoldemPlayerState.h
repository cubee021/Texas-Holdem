// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Item.h"
#include "HoldemPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class HOLDEM_API AHoldemPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AHoldemPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
public:
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
	// Waiting Phase에서 선택한 아이템
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Item")
	EItemType SelectedItem;

	UFUNCTION(Server, Reliable)
	void Server_SetSelectedItem(EItemType NewItem);
};
