// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SessionWidget.generated.h"

/**
 * 
 */
UCLASS()
class HOLDEM_API USessionWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

public:
	// 세션 이름
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Txt_SessionName;
	// 현대 참여 인원 수
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Txt_CurrentPlayers;
	// 참여 버튼
	UPROPERTY(meta = (BindWidget))
	class UButton* Btn_JoinSession;
	
	UFUNCTION()
	void OnClickJoinSession();
	
public:
	// 세션 목록 순번
	int32 SessionIdx;
	
	void SetSessionName(int32 Idx, FString SessionName);
	void SetSessionPlayersInfo(int32 CurrPlayers, int32 MaxPlayers);
};
