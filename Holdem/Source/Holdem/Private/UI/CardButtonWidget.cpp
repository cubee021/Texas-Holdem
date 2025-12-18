// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/CardButtonWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UCardButtonWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	ApplySettings();
}

void UCardButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Card)
	{
		Btn_Card->OnClicked.AddDynamic(this, &UCardButtonWidget::OnButtonClicked);
	}
	
	ApplySettings();
}

void UCardButtonWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	if (Anim_Hover)
	{
		PlayAnimation(Anim_Hover);
	}
}

void UCardButtonWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	if (Anim_UnHover)
	{
		PlayAnimation(Anim_UnHover);
	}
}

void UCardButtonWidget::ApplySettings()
{
	if (Img_Icon && IconTexture)
	{
		Img_Icon->SetBrushFromTexture(IconTexture);
	}

	if (Txt_Title)
	{
		Txt_Title->SetText(TitleText);
		Txt_Title->SetColorAndOpacity(TitleColor);
	}

	if (Btn_Card && ButtonBackgroundTexture)
	{
		FButtonStyle ButtonStyle = Btn_Card->GetStyle();

		FSlateBrush NewBrush;
		NewBrush.SetResourceObject(ButtonBackgroundTexture);

		ButtonStyle.Normal = NewBrush;
		ButtonStyle.Hovered = NewBrush;
		ButtonStyle.Pressed = NewBrush;
	}
}

void UCardButtonWidget::OnButtonClicked()
{
	OnCardButtonClicked.Broadcast();
}
