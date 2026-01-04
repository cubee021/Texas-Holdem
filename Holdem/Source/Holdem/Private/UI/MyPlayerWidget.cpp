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

	// PlayerState 델리게이트 구독 및 초기 UI 설정
	GetWorld()->GetTimerManager().SetTimer(DeligateTimerHandle, this,
		&UMyPlayerWidget::TrySubscribeDeligates, 0.1f, true);

	OnBettingSelectionChanged(1);
	UpdateButtonTexts();
}

void UMyPlayerWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	UpdateTimer();
}

void UMyPlayerWidget::TrySubscribeDeligates()
{
	AHoldemPlayerState* PS = GetOwningPlayerState<AHoldemPlayerState>();
	if (PS)
	{
		PS->OnBettingInfoChanged.AddDynamic(this, &UMyPlayerWidget::UpdateBettingInfo);
		PS->OnCurrentChipsChanged.AddDynamic(this, &UMyPlayerWidget::UpdateCurrentChips);
		PS->OnPositionChanged.AddDynamic(this, &UMyPlayerWidget::UpdatePosition);
		PS->OnFoldChanged.AddDynamic(this, &UMyPlayerWidget::UpdateHandRank);
		PS->OnSpectatingChanged.AddDynamic(this, &UMyPlayerWidget::UpdateSpectating);

		PS->OnSteamInfoChanged.AddDynamic(this, &UMyPlayerWidget::UpdateSteamInfo);
		
		// 초기 호출
		UpdatePlayerName();
		UpdateCurrentChips(PS->GetCurrentChips());
		UpdatePosition(PS->GetPosition());
		UpdateBettingInfo(PS->GetCurrentBet(), PS->GetTotalBet());
		UpdateHandRank(PS->GetIsFolded());
		UpdateSpectating(PS->GetIsSpectating());

		UpdateBettingUIVisibility();

		GetWorld()->GetTimerManager().ClearTimer(DeligateTimerHandle);

		//UE_LOG(LogTemp, Warning, TEXT("[Widget] Delegates subscribed for %s"), *PS->GetPlayerName());
	}
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
	int32 CallAmount = FMath::Max(0, GS->CurrentMaxBet - PS->GetCurrentBet());

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
		int32 RaiseAdditional = NewMaxBet - PS->GetCurrentBet();
		FString RaiseText = FString::Printf(TEXT("Raise %d"), RaiseAdditional);
		Txt_0->SetText(FText::FromString(RaiseText));
	}

	// Button 1: Call or Check
	if (CallAmount == 0)
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

void UMyPlayerWidget::UpdateBettingInfo(int32 NewCurrentBet, int32 NewTotalBet)
{
	if (Txt_CurrentBet)
		Txt_CurrentBet->SetText(FText::AsNumber(NewCurrentBet));

	if (Txt_TotalBet)
		Txt_TotalBet->SetText(FText::AsNumber(NewTotalBet));
}

void UMyPlayerWidget::UpdatePlayerName()
{
	// 초기 호출용
	AHoldemPlayerState* PS = GetOwningPlayerState<AHoldemPlayerState>();
	if (!PS || !Txt_PlayerName) return;

	// Steam 이름 사용, 비어있으면 기본 이름 사용                                                                                                                     
	FString DisplayName = PS->GetSteamName();
	if (DisplayName.IsEmpty())
	{
		DisplayName = PS->GetPlayerName();
	}

	// 이름을 33자로 제한
	FString TrimmedName = DisplayName.Left(33);
	Txt_PlayerName->SetText(FText::FromString(TrimmedName));
}

void UMyPlayerWidget::UpdateSteamInfo(FString InSteamID, FString InSteamName)
{
	if (Txt_PlayerName)
		Txt_PlayerName->SetText(FText::FromString(InSteamName));
}

void UMyPlayerWidget::UpdateCurrentChips(int32 NewChips)
{
	//UE_LOG(LogTemp, Warning, TEXT("[Widget] UpdateCurrentChips: %d"), NewChips);
	
	if (Txt_CurrentChips)
		Txt_CurrentChips->SetText(FText::AsNumber(NewChips));
}

void UMyPlayerWidget::UpdatePosition(EPlayerPosition NewPosition)
{
	if (!Txt_Position) return;

	FString PositionText;
	switch (NewPosition)
	{
	case EPlayerPosition::Dealer:
		PositionText = TEXT("Dealer");
		break;
	case EPlayerPosition::SmallBlind:
		PositionText = TEXT("Small Blind");
		break;
	case EPlayerPosition::BigBlind:
		PositionText = TEXT("Big Blind");
		break;
	default:
		PositionText = TEXT("-");
		break;
	}

	Txt_Position->SetText(FText::FromString(PositionText));
}

void UMyPlayerWidget::UpdateHandRank(bool bNewIsFolded)
{
	// 추후 족보 표시 예정 (지금은 Fold & Spectating 여부만)

	AHoldemPlayerState* PS = GetOwningPlayerState<AHoldemPlayerState>();
	if (PS && PS->GetIsSpectating()) return;
	
	if (bNewIsFolded)
	{
		Txt_HandRank->SetText(FText::FromString("Folded"));
		Txt_HandRank->SetColorAndOpacity(FLinearColor(1.0f, 0.5f, 0.5f, 1.0f));
	}
	else
	{
		Txt_HandRank->SetText(FText::GetEmpty());
		Txt_HandRank->SetColorAndOpacity(FLinearColor::White);
	}
}

void UMyPlayerWidget::UpdateSpectating(bool bNewIsSpectating)
{
	if (!Txt_HandRank) return;

	if (bNewIsSpectating)
	{
		Txt_HandRank->SetText(FText::FromString("Spectating"));
		Txt_HandRank->SetColorAndOpacity(FLinearColor(0.2f, 0.5f, 1.0f, 1.0f));
	}
	else
	{
		// Playing으로 전환되면 원래대로 복구
		Txt_HandRank->SetText(FText::GetEmpty());
		Txt_HandRank->SetColorAndOpacity(FLinearColor::White);
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
