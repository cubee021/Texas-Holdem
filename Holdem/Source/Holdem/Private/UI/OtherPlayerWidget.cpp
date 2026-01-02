// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/OtherPlayerWidget.h"

#include "Components/TextBlock.h"

void UOtherPlayerWidget::SetName(FString InName)
{
	// 이름을 33자로 제한
	FString TrimmedName = InName.Left(33);
	Txt_Name->SetText(FText::FromString(TrimmedName));
}
