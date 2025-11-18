// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BarWidget.h"

#include "Components/Button.h"

void UBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Btn_Back->OnClicked.AddDynamic(this, &UBarWidget::OnClicked_Back);
}

void UBarWidget::OnClicked_Back()
{
	
}
