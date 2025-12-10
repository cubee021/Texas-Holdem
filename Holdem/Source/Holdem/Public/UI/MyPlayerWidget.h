// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyPlayerWidget.generated.h"

/**
 * 
 */
UCLASS()
class HOLDEM_API UMyPlayerWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry & MyGeometry, float InDeltaTime) override;

	class AHoldemGameState* GS;
	
public:
	//---------------------------------------------------//
	// Timer
	//---------------------------------------------------//
	
	// Waiting Phase동안 돌아가는 타이머
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Txt_Timer;

	void UpdateTimer();

public:
	//---------------------------------------------------//
	// Betting UI
	//---------------------------------------------------//
	UPROPERTY(Meta = (BindWidget))
	class UVerticalBox* VtclBox_Betting;

	UPROPERTY(Meta = (BindWidget))
	class UButton* Btn_0;

	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* Txt_0;

	UPROPERTY(Meta = (BindWidget))
	class UButton* Btn_1;

	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* Txt_1;

	UPROPERTY(Meta = (BindWidget))
	class UButton* Btn_2;

	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* Txt_2;

	UFUNCTION()
	void OnBettingSelectionChanged(int32 SelectedIndex);

	UFUNCTION(BlueprintCallable)
	void UpdateButtonTexts();
};
