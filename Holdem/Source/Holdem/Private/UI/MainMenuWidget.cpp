// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MainMenuWidget.h"

#include "MyPlayerSaveGame.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Game/HoldemGameInstance.h"
#include "Kismet/GameplayStatics.h"
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
	
	// SignUpCanvas
	Btn_Confirm->OnClicked.AddDynamic(this, &UMainMenuWidget::OnClicked_Confirm);

	// MainCanvas
	CardBtn_Host->OnCardButtonClicked.AddUObject(this, &UMainMenuWidget::OnClicked_Host);
	CardBtn_Join->OnCardButtonClicked.AddUObject(this, &UMainMenuWidget::OnClicked_Join);

	// JoinCanvas
	Btn_Update->OnClicked.AddDynamic(this, &UMainMenuWidget::OnClicked_Update);
	GI->onFindComplete.BindUObject(this, &UMainMenuWidget::OnFindComplete);
	
	// 세이브 파일 확인
	UMyPlayerSaveGame* SG = Cast<UMyPlayerSaveGame>(
		UGameplayStatics::LoadGameFromSlot(TEXT("PlayerDataSlot"), 0));

	// 만약 세이브 파일 없거나 이름 누락이면 SignUpCanvas로, 있으면 바로 메인으로
	if (SG && !SG->PlayerName.IsEmpty())
	{
		WidgetSwitcher->SetActiveWidgetIndex(1);
	}
	else
	{
		WidgetSwitcher->SetActiveWidgetIndex(0);
	}
}

void UMainMenuWidget::OnClicked_Confirm()
{
	FString InputName = EdtTxt_Name->GetText().ToString();

	// 빈 문자열인지 체크
	if (InputName.IsEmpty()) return;

	// 세이브 파일 생성
	UMyPlayerSaveGame* NewSG = Cast<UMyPlayerSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UMyPlayerSaveGame::StaticClass()));

	// InputName을 save 파일에 로컬로 저장
	if (NewSG)
	{
		NewSG->PlayerName = InputName;

		// 저장 성공하면 MainCanvas로
		if (UGameplayStatics::SaveGameToSlot(NewSG, NewSG->GetSaveSlotName(), NewSG->GetUserIdx()))
		{
			WidgetSwitcher->SetActiveWidgetIndex(1);
		}
	}
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

	GI->CreateMySession(SessionName);
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
