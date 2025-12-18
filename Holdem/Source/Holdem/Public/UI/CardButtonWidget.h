// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CardButtonWidget.generated.h"

/**
 * 
 */
UCLASS()
class HOLDEM_API UCardButtonWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

public:
	UPROPERTY(meta = (BindWidget))
	class UButton* Btn_Card;

	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* Vtcl_Info;

	UPROPERTY(meta = (BindWidget))
	class UImage* Img_Icon;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Txt_Title;

public:
	//---------------------------------------------------//
	// Hover Animations
	//---------------------------------------------------//
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	class UWidgetAnimation* Anim_Hover;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	class UWidgetAnimation* Anim_UnHover;

public:
	//---------------------------------------------------//
	// Editable Properties
	//---------------------------------------------------//

	// 아이콘 이미지
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Card Button")
	UTexture2D* IconTexture;

	// 텍스트
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Card Button")
	FText TitleText;

	// 텍스트 색상
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Card Button")
	FSlateColor TitleColor;

	// 버튼 배경
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Card Button")
	UTexture2D* ButtonBackgroundTexture;

private:
	void ApplySettings();

private:
	//---------------------------------------------------//
	// Event Handler
	//---------------------------------------------------//

	UFUNCTION()
	void OnButtonClicked();

public:
	DECLARE_MULTICAST_DELEGATE(FOnCardButtonClicked);
	FOnCardButtonClicked OnCardButtonClicked;
};
