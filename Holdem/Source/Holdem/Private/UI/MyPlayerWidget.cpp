// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MyPlayerWidget.h"

#include "MyPlayerController.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "Game/HoldemGameState.h"
#include "Game/HoldemPlayerState.h"
#include "Kismet/GameplayStatics.h"

void UMyPlayerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GS = Cast<AHoldemGameState>(UGameplayStatics::GetGameState(this));

	AMyPlayerController* PC = Cast<AMyPlayerController>(GetOwningPlayer());
	if (PC)
	{
		PC->OnBettingSelectionChanged.AddDynamic(this, &UMyPlayerWidget::OnBettingSelectionChanged);
	}

	if (GS)
	{
		GS->OnTurnChanged.AddDynamic(this, &UMyPlayerWidget::OnTurnChangedHandler);
	}

	OnBettingSelectionChanged(1);
	UpdateButtonTexts();
}

void UMyPlayerWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	UpdateTimer();
}

void UMyPlayerWidget::UpdateTimer()
{
	if (!GS) return;
	
	if (Txt_Timer)
	{
		Txt_Timer->SetText(FText::FromString(
			FString::Printf(TEXT("%d"), (int32)GS->WaitingTimeRemaining)));
	}
}

void UMyPlayerWidget::UpdateButtonTexts()
{
	AHoldemPlayerState* PS = GetOwningPlayerState<AHoldemPlayerState>();
	if (!PS || !GS) return;

	// Fixed Limit : 현재 Phase에 따른 베팅 단위
	int32 BettingUnit = (GS->CurrentPhase == EHoldemPhase::Turn ||
						GS->CurrentPhase == EHoldemPhase::River)
						? GS->BigBetAmount : GS->SmallBetAmount;
	
	// 베팅 상황 체크
	bool bNoBets = (GS->CurrentMaxBet == 0);
	bool bCanCheck = (PS->CurrentBet == GS->CurrentMaxBet);
	int32 CallAmount = FMath::Max(0, GS->CurrentMaxBet - PS->CurrentBet);

	// Button 0: Raise of Bet
	if (bNoBets)
	{
		// 아무도 베팅 안했으면 "Bet" (Flop/Turn/River에서만 가능)  
		FString BetText = FString::Printf(TEXT("Bet %d"), BettingUnit);
		Txt_0->SetText(FText::FromString(BetText));
	}
	else
	{
		// 누군가 베팅 했으면 "Raise"
		int32 NewMaxBet = GS->CurrentMaxBet + BettingUnit;
		int32 RaiseAdditional = NewMaxBet - PS->CurrentBet;
		FString RaiseText = FString::Printf(TEXT("Raise %d"), RaiseAdditional);
		Txt_0->SetText(FText::FromString(RaiseText));
	}

	// Button 1: Call or Check
	if (bCanCheck)
	{
		Txt_1->SetText(FText::FromString("Check"));
	}
	else
	{
		FString CallText = FString::Printf(TEXT("Call %d"), CallAmount);
		Txt_1->SetText(FText::FromString(CallText));
	}

	// Button 2: Fold
	Txt_2->SetText(FText::FromString("Fold"));
}

void UMyPlayerWidget::OnTurnChangedHandler(int32 NewTurnIndex)
{
	// UI 표시/숨김 업데이트
	UpdateBettingUIVisibility();

	AMyPlayerController* PC = Cast<AMyPlayerController>(GetOwningPlayer());
	if (PC && PC->IsMyTurn())
	{
		// ✅ PlayerController의 SelectedButtonIndex를 1로 리셋!
		PC->SelectedButtonIndex = 1;
		
		OnBettingSelectionChanged(1);
		UpdateButtonTexts();
	}
}

void UMyPlayerWidget::OnBettingSelectionChanged(int32 SelectedIndex)
{
	// 모든 버튼 기본 스타일
	Btn_0->SetBackgroundColor(FLinearColor::Black);
	Btn_1->SetBackgroundColor(FLinearColor::Black);
	Btn_2->SetBackgroundColor(FLinearColor::Black);

	Txt_0->SetColorAndOpacity(FLinearColor::Gray);
	Txt_1->SetColorAndOpacity(FLinearColor::Gray);
	Txt_2->SetColorAndOpacity(FLinearColor::Gray);

	// 선택된 버튼 강조                                                                                                                                             
	switch (SelectedIndex)
	{
	case 0: Btn_0->SetBackgroundColor(FLinearColor(0, 0, 0, 0)); break;
	case 1: Btn_1->SetBackgroundColor(FLinearColor(0, 0, 0, 0)); break;
	case 2: Btn_2->SetBackgroundColor(FLinearColor(0, 0, 0, 0)); break;
	}

	switch (SelectedIndex)
	{
	case 0: Txt_0->SetColorAndOpacity(FLinearColor(1, 1, 1, 1)); break;
	case 1: Txt_1->SetColorAndOpacity(FLinearColor(1, 1, 1, 1)); break;
	case 2: Txt_2->SetColorAndOpacity(FLinearColor(1, 1, 1, 1)); break;
	}
}

void UMyPlayerWidget::UpdateBettingUIVisibility()
{
	if (!HBox_Buttons) return;

	AMyPlayerController* PC = Cast<AMyPlayerController>(GetOwningPlayer());
	if (!PC) return;

	bool bShouldShowUI = PC->IsMyTurn() && GS &&
		(GS->CurrentPhase == EHoldemPhase::PreFlop ||
		GS->CurrentPhase == EHoldemPhase::Flop ||
		GS->CurrentPhase == EHoldemPhase::Turn ||
		GS->CurrentPhase == EHoldemPhase::River);

	HBox_Buttons->SetVisibility(
		bShouldShowUI ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}
