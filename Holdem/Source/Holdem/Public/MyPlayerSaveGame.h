// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "MyPlayerSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class HOLDEM_API UMyPlayerSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UMyPlayerSaveGame();

	UPROPERTY(VisibleAnywhere, Category = "PlayerData")
	FString PlayerName;

	// 저장 및 로드에 사용할 슬롯 이름
	static FString GetSaveSlotName() {return TEXT("PlayerDataSlot");}
	// 같은 PC에서의 멀티유저 구분용
	static int32 GetUserIdx() {return 0;}
};
