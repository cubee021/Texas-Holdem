// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SessionWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

void USessionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Btn_JoinSession->OnClicked.AddDynamic(this, &USessionWidget::OnClickJoinSession);
	
}

void USessionWidget::OnClickJoinSession()
{
	//HoldemGameInstance* GI = Cast<HoldemGameInstance>(GetGameInstance());
	// sessionIdx 번째 세션에 참여
	//GI->JoinOtherSession(sessionIdx);
}

void USessionWidget::SetSessionName(int32 Idx)
{
	// 업데이트해도 인덱스 동일한지 확인할 것
	SessionIdx = Idx;
	
	Txt_SessionName->SetText(FText::FromString(FString::Printf(TEXT("%d"), Idx)));
}

void USessionWidget::SetSessionPlayersInfo(int32 CurrPlayers, int32 MaxPlayers)
{
	Txt_CurrentPlayers->SetText(FText::FromString(
		FString::Printf(TEXT("%d / %d"), CurrPlayers, MaxPlayers)));
}
