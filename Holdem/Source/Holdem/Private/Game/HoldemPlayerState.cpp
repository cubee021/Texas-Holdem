// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/HoldemPlayerState.h"

#include "Net/UnrealNetwork.h"

AHoldemPlayerState::AHoldemPlayerState()
{
	SelectedItem = EItemType::None;
}

void AHoldemPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHoldemPlayerState, SelectedItem);
}

void AHoldemPlayerState::AddCard(ACard* Card)
{
	if (Card) HandCards.Add(Card);
}

void AHoldemPlayerState::ClearHand()
{
	HandCards.Empty();
}
