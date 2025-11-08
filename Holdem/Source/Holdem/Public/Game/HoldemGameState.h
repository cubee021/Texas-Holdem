// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "HoldemGameState.generated.h"

/**
 * 
 */
UCLASS()
class HOLDEM_API AHoldemGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AHoldemGameState();

public:
	// 현재 접속한 플레이어 수
	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetPlayerCount() const {return PlayerArray.Num();}

	// 최대 플레이어 수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxPlayers = 4;
};
