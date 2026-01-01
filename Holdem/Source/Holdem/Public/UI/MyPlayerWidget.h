// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyPlayerWidget.generated.h"

/**
 * 
 */
UCLASS()
class HOLDEM_API UMyPlayerWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry & MyGeometry, float InDeltaTime) override;

	class AHoldemGameState* GS;

	// PlayerState deligate 지연 구독
	FTimerHandle DeligateTimerHandle;
	void TrySubscribeDeligates();
	
public:
	//---------------------------------------------------//
	// Timer
	//---------------------------------------------------//
	
	// Waiting Phase동안 돌아가는 타이머
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Txt_Timer;

	void UpdateTimer();

public:
	//---------------------------------------------------//
	// Betting UI
	//---------------------------------------------------//
	/* Button */
	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* HBox_Buttons;

	UPROPERTY(Meta = (BindWidget))
	class UButton* Btn_0;

	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* Txt_0;

	UPROPERTY(Meta = (BindWidget))
	class UButton* Btn_1;

	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* Txt_1;

	UPROPERTY(Meta = (BindWidget))
	class UButton* Btn_2;

	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* Txt_2;

	// 선택한 버튼 색깔 변경
	UFUNCTION()
	void OnBettingSelectionChanged(int32 SelectedIndex);

	UFUNCTION(BlueprintCallable)
	void UpdateBettingUIVisibility();
	
	UFUNCTION(BlueprintCallable)
	void UpdateButtonTexts();

	// 턴 변경 이벤트 핸들러
	// (플레이어 베팅 턴 바뀔 때마다 호출)
	UFUNCTION()
	void OnTurnChangedHandler(int32 NewTurnIndex);

	/* Text */
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Txt_CurrentBet;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Txt_TotalBet;

	UFUNCTION()
	void UpdateBettingInfo(int32 NewCurrentBet, int32 NewTotalBet);

public:
	//---------------------------------------------------//
	// Player Info UI
	//---------------------------------------------------//
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Txt_PlayerName;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Txt_CurrentChips;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Txt_Position;
	// Fold 표시 (추후에 족보 표시 예정)
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Txt_HandRank;

	UFUNCTION()
	void UpdatePlayerName();

	UFUNCTION()
	void UpdateSteamInfo(FString InSteamID, FString InSteamName);

	UFUNCTION()
	void UpdateCurrentChips(int32 NewChips);

	UFUNCTION()
	void UpdatePosition(EPlayerPosition NewPosition);
	
	UFUNCTION()
	void UpdateHandRank(bool bNewIsFolded);
	
};
