// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MainMenuWidget.h"

#include "MyPlayerSaveGame.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/WidgetSwitcher.h"
#include "Game/HoldemGameInstance.h"
#include "Kismet/GameplayStatics.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	GI = Cast<UHoldemGameInstance>(GetGameInstance());
	
	// SignUpCanvas
	Btn_Confirm->OnClicked.AddDynamic(this, &UMainMenuWidget::OnClicked_Confirm);

	// MainCanvas
	Btn_Host->OnClicked.AddDynamic(this, &UMainMenuWidget::OnClicked_Host);
	Btn_Join->OnClicked.AddDynamic(this, &UMainMenuWidget::OnClicked_Join);

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

void UMainMenuWidget::OnClicked_Host()
{
	// 플레이어명과 동일하게 세션 생성
	UMyPlayerSaveGame* SG = Cast<UMyPlayerSaveGame>(
		UGameplayStatics::LoadGameFromSlot(TEXT("PlayerDataSlot"), 0));

	if (!SG) return;

	FString SessionName = SG->PlayerName;

	GI->CreateMySession(SessionName);
}

void UMainMenuWidget::OnClicked_Join()
{
	
}
