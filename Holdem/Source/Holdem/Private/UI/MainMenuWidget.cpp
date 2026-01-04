// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MainMenuWidget.h"

#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Game/HoldemGameInstance.h"
#include "UI/CardButtonWidget.h"
#include "UI/SessionWidget.h"

class USessionWidget;

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	GI = Cast<UHoldemGameInstance>(GetGameInstance());
	if (GI)
	{
		FString SteamName = GI->LocalPlayerSteamName;
		UpdateSteamName(SteamName);
	}

	// 마우스 커서 보이게
	GetWorld()->GetFirstPlayerController()->SetShowMouseCursor(true);
	
	// MainCanvas
	CardBtn_Host->OnCardButtonClicked.AddUObject(this, &UMainMenuWidget::OnClicked_Host);
	CardBtn_Join->OnCardButtonClicked.AddUObject(this, &UMainMenuWidget::OnClicked_Join);

	// JoinCanvas
	Btn_Update->OnClicked.AddDynamic(this, &UMainMenuWidget::OnClicked_Update);
	GI->onFindComplete.BindUObject(this, &UMainMenuWidget::OnFindComplete);

	// Init
	WidgetSwitcher->SetActiveWidgetIndex(0);
}

void UMainMenuWidget::UpdateSteamName(FString InName)
{
	if (Txt_SteamName)
		Txt_SteamName->SetText(FText::FromString(InName));
}

void UMainMenuWidget::OnClicked_Host()
{
	// 플레이어명과 동일하게 세션 생성
	if (!GI) return;

	FString SessionName = GI->LocalPlayerSteamName;
	if (SessionName.IsEmpty())
	{
		SessionName = TEXT("Unknown");
	}

	// 이름을 33자로 제한
	FString TrimmedName = SessionName.Left(33);
	GI->CreateMySession(TrimmedName);
}

void UMainMenuWidget::OnClicked_Join()
{
	// JoinCanvas로 이동
	WidgetSwitcher->SetActiveWidgetIndex(2);

	// SessionWidget 최초 업데이트
	OnClicked_Update();
}

void UMainMenuWidget::OnClicked_Update()
{
	ScrlBox_SessionList->ClearChildren();
	// 세션 검색 중 버튼 비활성화
	Txt_Update->SetText(FText::FromString(TEXT("Updating...")));
	Btn_Update->SetIsEnabled(false);

	GI->FindOtherSession();
}

void UMainMenuWidget::OnFindComplete(int Idx, FString SessionName,
	int32 CurrPlayers, int32 MaxPlayers)
{
	if (Idx == -1)
	{
		// 더 검색된 세션 없으면 검색 버튼 활성화
		Txt_Update->SetText(FText::FromString(TEXT("Update Session")));
		Btn_Update->SetIsEnabled(true);
	}
	else
	{
		// 검색된 세션 있으면 리스트에 추가
		USessionWidget* NewSession = CreateWidget<USessionWidget>(GetWorld(), SessionWidget);
		if (NewSession)
		{
			ScrlBox_SessionList->AddChild(NewSession);
			NewSession->SetSessionName(Idx, SessionName);
			NewSession->SetSessionPlayersInfo(CurrPlayers, MaxPlayers);
		}
	}
}
