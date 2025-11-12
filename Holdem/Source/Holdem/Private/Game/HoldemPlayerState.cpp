// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/HoldemPlayerState.h"

AHoldemPlayerState::AHoldemPlayerState()
{
}

void AHoldemPlayerState::AddCard(ACard* Card)
{
	if (Card) HandCards.Add(Card);
}

void AHoldemPlayerState::ClearHand()
{
	HandCards.Empty();
}
