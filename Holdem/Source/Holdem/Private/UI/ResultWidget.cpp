// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ResultWidget.h"

#include "Components/TextBlock.h"

void UResultWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UResultWidget::UpdateResult(FString WinnerNames, EHandRank HighestRank)
{
	Txt_Winner->SetText(FText::FromString(WinnerNames));

	// TODO: 이미지 업데이트는 나중에
}

  