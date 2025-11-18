// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BarWidget.generated.h"

/**
 * 
 */
UCLASS()
class HOLDEM_API UBarWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	void NativeConstruct() override;

public:
	// Info
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Txt_Name;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Txt_SessionName;

public:
	// Menu
	UPROPERTY(meta = (BindWidget))
	class UCheckBox* CkBox_Cocktails1;

	UPROPERTY(meta = (BindWidget))
	class UCheckBox* CkBox_Cocktails2;

	UPROPERTY(meta = (BindWidget))
	class UCheckBox* CkBox_Whiskey1;

	UPROPERTY(meta = (BindWidget))
	class UCheckBox* CkBox_Cigar;

	UPROPERTY(meta = (BindWidget))
	class UCheckBox* CkBox_None;

public:
	UPROPERTY(meta = (BindWidget))
	class UButton* Btn_Back;

	UFUNCTION()
	void OnClicked_Back();
};
