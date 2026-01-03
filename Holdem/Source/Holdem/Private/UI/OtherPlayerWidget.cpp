// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/OtherPlayerWidget.h"

#include "Components/TextBlock.h"
#include "Game/HoldemPlayerState.h"

void UOtherPlayerWidget::GetOwningPlayerState(class AHoldemPlayerState* PS)
{
	if (PS)
	{
		PS->OnCurrentChipsChanged.AddDynamic(this, &UOtherPlayerWidget::UOtherPlayerWidget::UpdateCurrentChips);
		PS->OnBettingInfoChanged.AddDynamic(this, &UOtherPlayerWidget::UpdateBettingInfo);
		PS->OnPositionChanged.AddDynamic(this, &UOtherPlayerWidget::UpdatePosition);
		PS->OnFoldChanged.AddDynamic(this, &UOtherPlayerWidget::UpdateIsFold);
		PS->OnSpectatingChanged.AddDynamic(this, &UOtherPlayerWidget::UpdateSpectating);

		// 초기값 설정
		FString DisplayName = PS->GetSteamName();
		if (DisplayName.IsEmpty())
		{
			DisplayName = PS->GetPlayerName();
		}
		SetName(DisplayName);
		
		UpdateCurrentChips(PS->GetCurrentChips());
		UpdateBettingInfo(PS->GetCurrentBet(), PS->GetTotalBet());
		UpdatePosition(PS->GetPosition());
		UpdateIsFold(PS->GetIsFolded());
		UpdateSpectating(PS->GetIsSpectating());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[OtherPlayerWidget] PlayerState is NULL!"));
	}
}

void UOtherPlayerWidget::SetName(FString InName)
{
	// 이름을 33자로 제한
	FString TrimmedName = InName.Left(33);
	Txt_Name->SetText(FText::FromString(TrimmedName));
}

void UOtherPlayerWidget::UpdateCurrentChips(int32 NewChips)
{
	if (Txt_CurrentChips)
		Txt_CurrentChips->SetText(FText::AsNumber(NewChips));
}

void UOtherPlayerWidget::UpdateBettingInfo(int32 NewCurrentBet, int32 NewTotalBet)
{
	if (Txt_CurrentBet)
		Txt_CurrentBet->SetText(FText::AsNumber(NewCurrentBet));

	if (Txt_TotalBet)
		Txt_TotalBet->SetText(FText::AsNumber(NewTotalBet));
}

void UOtherPlayerWidget::UpdatePosition(EPlayerPosition NewPosition)
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
		PositionText = TEXT("");
		break;
	}

	Txt_Position->SetText(FText::FromString(PositionText));
	Txt_Position->SetColorAndOpacity(FLinearColor::White);
}

void UOtherPlayerWidget::UpdateIsFold(bool bNewIsFolded)
{
	if (!Txt_Position) return;

	// Fold 상태면 Position 텍스트를 "Fold"로 변경
	if (bNewIsFolded)
	{
		Txt_Position->SetText(FText::FromString("Fold"));
		Txt_Position->SetColorAndOpacity(FLinearColor(1.0f, 0.5f, 0.5f, 1.0f)); 
	}
}

void UOtherPlayerWidget::UpdateSpectating(bool bNewIsSpectating)
{
	if (!Txt_Position) return;

	if (bNewIsSpectating)
	{
		Txt_Position->SetText(FText::FromString("Spectating"));
		Txt_Position->SetColorAndOpacity(FLinearColor(0.2f, 0.5f, 1.0f, 1.0f));
	}
	else
	{
		// Playing으로 전환되면 원래대로 복구
		Txt_Position->SetText(FText::GetEmpty());
		Txt_Position->SetColorAndOpacity(FLinearColor::White);
	}
}
