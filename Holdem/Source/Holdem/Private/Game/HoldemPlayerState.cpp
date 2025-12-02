// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/HoldemPlayerState.h"

#include "MyPlayerSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

AHoldemPlayerState::AHoldemPlayerState()
{
	SelectedItem = EItemType::None;

	// 플레이어 이름 설정(이거 뭔가 잘 안됨)
	UMyPlayerSaveGame* SG = Cast<UMyPlayerSaveGame>(
		UGameplayStatics::LoadGameFromSlot(TEXT("PlayerDataSlot"), 0));
	if (SG)
	{
		SetPlayerName(SG->PlayerName);
	}

	bIsFolded = false;
	Position = EPlayerPosition::None;
	CurrentChips = 1000;
}

void AHoldemPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHoldemPlayerState, SelectedItem);

	DOREPLIFETIME(AHoldemPlayerState, bIsFolded);
	DOREPLIFETIME(AHoldemPlayerState, Position);

	DOREPLIFETIME(AHoldemPlayerState, CurrentChips);
}

void AHoldemPlayerState::AddCard(ACard* Card)
{
	if (Card) HandCards.Add(Card);
}

void AHoldemPlayerState::ClearHand()
{
	HandCards.Empty();
}

void AHoldemPlayerState::Server_SetSelectedItem_Implementation(EItemType NewItem)
{
	SelectedItem = NewItem;

	UE_LOG(LogTemp, Warning, TEXT("Player %s selected item: %d"),
		*GetPlayerName(), (int32)NewItem);
}
