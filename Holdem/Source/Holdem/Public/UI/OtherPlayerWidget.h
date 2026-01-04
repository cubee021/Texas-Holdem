// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OtherPlayerWidget.generated.h"

/**
 * 상대 플레이어에게 보여지는 자신의 정보
 * 이 UI는 자기 자신에게는 안보임
 */
UCLASS()
class HOLDEM_API UOtherPlayerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void GetOwningPlayerState(class AHoldemPlayerState* PS);
	
public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Txt_Name;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Txt_CurrentChips;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Txt_CurrentBet;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Txt_TotalBet;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Txt_Position;

	void SetName(FString InName);

	UFUNCTION()
	void UpdateCurrentChips(int32 NewChips);

	UFUNCTION()
	void UpdateBettingInfo(int32 NewCurrentBet, int32 NewTotalBet);

	UFUNCTION()
	void UpdatePosition(EPlayerPosition NewPosition);

	UFUNCTION()
	void UpdateIsFold(bool bNewIsFolded);

	UFUNCTION()
	void UpdateSpectating(bool bNewIsSpectating);

public:
	// 내가 누른 베팅 버튼이 뭔지 알려줌
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Txt_BettingState;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	class UWidgetAnimation* FadeOut;

	UFUNCTION()
	void ShowBettingAction(FString BettingAction);
};
