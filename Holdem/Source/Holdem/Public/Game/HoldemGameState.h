// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Card.h"
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

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
public:
	// 현재 접속한 플레이어 수
	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetPlayerCount() const {return PlayerArray.Num();}

	// 최대 플레이어 수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxPlayers = 4;

public:
	// Deck Management
	// 덱 (52장 카드 데이터)
	UPROPERTY(BlueprintReadOnly, Category = "Deck")
	TArray<FCardData> Deck;
	// 맵에 스폰된 카드 액터들
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Deck")
	TArray<ACard*> SpawnedCards;
	// 카드 블루프린트 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Deck")
	TSubclassOf<ACard> CardClass;

	// 덱 생성
	UFUNCTION(BlueprintCallable, Category = "Deck")
	void GenerateDeck();
	// 덱 셔플
	UFUNCTION(BlueprintCallable, Category = "Deck")
	void ShuffleDeck();
	// 카드 맵에 스폰 (데이터 -> 액터)
	UFUNCTION(BlueprintCallable, Category = "Deck")
	ACard* SpawnCard(const FCardData& Data, FVector Location, FRotator Rotation);
	// 카드 스폰- 테스트용
	UFUNCTION(BlueprintCallable, Category = "Deck")
	void SpawnTest();
	// 모든 카드 원위치
	UFUNCTION(BlueprintCallable, Category = "Deck")
	void ResetAllCardsLocation();
	
};
