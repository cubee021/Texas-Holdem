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

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
public:
	// Waiting - 10초 동안 플레이어 접속 대기
	UFUNCTION(BlueprintCallable, Category = "GamePhase")
	void StartWaiting();

	FTimerHandle WaitingTimerHandle;
	void UpdateWaitingTimer();
	
	// PreFlop - 플레이어당 2장씩 카드 배분
	UFUNCTION(BlueprintCallable, Category = "GamePhase")
	void StartPreFlop();
	// Flop - 테이블 중앙에 3장 배분
	UFUNCTION(BlueprintCallable, Category = "GamePhase")
	void StartFlop();
	// Turn - 테이블 중앙에 4번째 카드 배분
	UFUNCTION(BlueprintCallable, Category = "GamePhase")
	void StartTurn();
	// River - 테이블 중앙에 5번째 카드 배분
	UFUNCTION(BlueprintCallable, Category = "GamePhase")
	void StartRiver();

	//Showdown 구현은 추후에
};
