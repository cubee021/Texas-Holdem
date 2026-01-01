// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class HOLDEM_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	UPROPERTY()
	class UHoldemGameInstance* GI;
	
	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* WidgetSwitcher;

public:
	// 0 : SignUpCanvas
	UPROPERTY(meta = (BindWidget))
	class UEditableTextBox* EdtTxt_Name;

	UPROPERTY(meta = (BindWidget))
	class UButton* Btn_Confirm;

	UFUNCTION()
	void OnClicked_Confirm();

public:
	// 1 : MainCanvas
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Txt_SteamName;
	
	UPROPERTY(meta = (BindWidget))
	class UCardButtonWidget* CardBtn_Host;

	UPROPERTY(meta = (BindWidget))
	class UCardButtonWidget* CardBtn_Join;

	UFUNCTION()
	void UpdateSteamName(FString InName);

	UFUNCTION()
	void OnClicked_Host();
	
	UFUNCTION()
	void OnClicked_Join();

public:
	// 2 : JoinCanvas
	UPROPERTY(meta = (BindWidget))
	class UScrollBox* ScrlBox_SessionList;
	
	UPROPERTY(meta=(BindWidget))
	class UButton* Btn_Update;
	
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* Txt_Update;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> SessionWidget;

	UFUNCTION()
	void OnClicked_Update();

	UFUNCTION()
	void OnFindComplete(int Idx, FString SessionName,
		int32 CurrPlayers, int32 MaxPlayers);
};
