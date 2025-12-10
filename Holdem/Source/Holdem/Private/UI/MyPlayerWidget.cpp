// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MyPlayerWidget.h"

#include "MyPlayerController.h"
#include "Components/Button.h"
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

	// Button 0: Raise or Bet                                                                                                                                       
	//Txt_0->SetText(bNoBets ? FText::FromString("Bet") : FText::FromString("Raise"));

	// Button 1: Call or Check                                                                                                                                      
	//Txt_1->SetText(bCanCheck ? FText::FromString("Check") : FText::FromString("Call"));

	// Button 2: Fold (항상 동일)                                                                                                                                   
	Txt_2->SetText(FText::FromString("Fold"));
}

void UMyPlayerWidget::OnBettingSelectionChanged(int32 SelectedIndex)
{
	// 모든 버튼 기본 스타일                                                                                                                                        
	Btn_0->SetBackgroundColor(FLinearColor::White);
	Btn_1->SetBackgroundColor(FLinearColor::White);
	Btn_2->SetBackgroundColor(FLinearColor::White);

	// 선택된 버튼 강조                                                                                                                                             
	switch (SelectedIndex)
	{
	case 0: Btn_0->SetBackgroundColor(FLinearColor::Yellow); break;
	case 1: Btn_1->SetBackgroundColor(FLinearColor::Yellow); break;
	case 2: Btn_2->SetBackgroundColor(FLinearColor::Yellow); break;
	}
}
