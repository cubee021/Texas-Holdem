// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/NameTagWidget.h"

#include "Components/TextBlock.h"

void UNameTagWidget::SetName(FString InName)
{
	Txt_Name->SetText(FText::FromString(InName));
}
