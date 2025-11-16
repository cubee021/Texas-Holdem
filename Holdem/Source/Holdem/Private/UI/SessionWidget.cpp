// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SessionWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Game/HoldemGameInstance.h"

void USessionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Btn_JoinSession->OnClicked.AddDynamic(this, &USessionWidget::OnClickJoinSession);
	
}

void USessionWidget::OnClickJoinSession()
{
	UHoldemGameInstance* GI = Cast<UHoldemGameInstance>(GetGameInstance());
	// SessionIdx 번째 세션에 참여
	GI->JoinOtherSession(SessionIdx);
}

void USessionWidget::SetSessionName(int32 Idx, FString SessionName)
{
	SessionIdx = Idx;
	
	Txt_SessionName->SetText(FText::FromString(SessionName));
}

void USessionWidget::SetSessionPlayersInfo(int32 CurrPlayers, int32 MaxPlayers)
{
	Txt_CurrentPlayers->SetText(FText::FromString(
		FString::Printf(TEXT("%d / %d"), CurrPlayers, MaxPlayers)));
}
