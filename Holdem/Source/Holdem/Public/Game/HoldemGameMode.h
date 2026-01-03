// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HoldemGameState.h"
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
	//---------------------------------------------------//
	// Login
	//---------------------------------------------------//
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
public:
	//---------------------------------------------------//
	// Holdem Phase Management
	//---------------------------------------------------//
	
	// Waiting - 10초 동안 플레이어 접속 대기
	UFUNCTION(BlueprintCallable, Category = "GamePhase")
	void StartWaiting();

	FTimerHandle WaitingTimerHandle;
	void UpdateWaitingTimer();

	// PreFlop 시작 시 Look 입력 차단용 타이머
	FTimerHandle LookDisableTimerHandle;
	
	UPROPERTY(EditDefaultsOnly, Category = "GamePhase")
	float LookDisableTime = 3.f;
	
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
	// Showdown - 승자 결정
	UFUNCTION(BlueprintCallable, Category = "GamePhase")
	void StartShowdown();

protected:
	// Change Phase helper : 상태 서버&클라이언트 일괄 변경
	void ChangeGamePhase(EHoldemPhase NewState);

public:
	//---------------------------------------------------//
	// Betting Phase Management
	//---------------------------------------------------//

	// 베팅 라운드 시작
	void StartBettingRound();
	// 베팅 라운드 종료 조건 체크
	bool IsBettingRoundComplete();
	// 다음 플레이어로 턴 이동
	void MoveToNextPlayer();

protected:
	//---------------------------------------------------//
	// Round Over
	//---------------------------------------------------//
	FTimerHandle NextRoundTimerHandle;

	void PrepareNextRound();
};
