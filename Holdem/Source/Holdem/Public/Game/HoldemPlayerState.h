// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
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

public:
	// 플레이어가 소유한 카드
	UPROPERTY(BlueprintReadOnly, Category = "Hand")
	TArray<ACard*> HandCards;

	// 카드 추가
	UFUNCTION(BlueprintCallable, Category = "Hand")
	void AddCard(ACard* Card);
	// 패 초기화
	UFUNCTION(BlueprintCallable, Category = "Hand")
	void ClearHand();
};
