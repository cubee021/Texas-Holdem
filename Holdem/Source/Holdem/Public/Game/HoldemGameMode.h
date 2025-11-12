// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HoldemGameMode.generated.h"

/**
 * 
 */
UCLASS()
class HOLDEM_API AHoldemGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AHoldemGameMode();

protected:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

public:
	// PreFlop : 플레이어에게 카드 배분
	UFUNCTION(BlueprintCallable, Category = "GamePhase")
	void PreFlop();
	
	
};
