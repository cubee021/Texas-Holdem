// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/HoldemPlayerState.h"

#include "Card.h"
#include "MyPlayerSaveGame.h"
#include "Game/HoldemGameInstance.h"
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

	DOREPLIFETIME(AHoldemPlayerState, SeatIndex);

	DOREPLIFETIME(AHoldemPlayerState, SelectedItem);

	DOREPLIFETIME(AHoldemPlayerState, bIsFolded);
	DOREPLIFETIME(AHoldemPlayerState, Position);

	DOREPLIFETIME(AHoldemPlayerState, CurrentChips);

	DOREPLIFETIME(AHoldemPlayerState, CurrentBet);
	DOREPLIFETIME(AHoldemPlayerState, TotalBet);

	DOREPLIFETIME(AHoldemPlayerState, SteamID);
	DOREPLIFETIME(AHoldemPlayerState, SteamName);
}

void AHoldemPlayerState::AddCard(ACard* Card)
{
	if (Card) HandCards.Add(Card);
}

void AHoldemPlayerState::ClearHand()
{
	if (!HasAuthority()) return;
	
	for (ACard* Card : HandCards)
	{
		if (Card&&!Card->IsPendingKillPending())
		{
			Card->Destroy();
		}
	}
	HandCards.Empty();
}

void AHoldemPlayerState::OnRep_SteamID()
{
	OnSteamInfoChanged.Broadcast(SteamID, SteamName);
}

void AHoldemPlayerState::OnRep_SteamName()
{
	OnSteamInfoChanged.Broadcast(SteamID, SteamName);
}

void AHoldemPlayerState::SetSteamID(const FString& NewSteamID)
{
	SteamID = NewSteamID;
	OnSteamInfoChanged.Broadcast(SteamID, SteamName);
}

void AHoldemPlayerState::SetSteamName(const FString& NewSteamName)
{
	SteamName = NewSteamName;
	OnSteamInfoChanged.Broadcast(SteamID, SteamName);
}

void AHoldemPlayerState::OnRep_bIsFolded()
{
	OnFoldChanged.Broadcast(bIsFolded);
}

void AHoldemPlayerState::SetIsFolded(bool bNewFolded)
{
	bIsFolded = bNewFolded;
	OnFoldChanged.Broadcast(bIsFolded);
}

void AHoldemPlayerState::SetPosition(EPlayerPosition NewPosition)
{
	Position = NewPosition;
	OnPositionChanged.Broadcast(Position);
}

void AHoldemPlayerState::OnRep_Position()
{
	OnPositionChanged.Broadcast(Position);
}

void AHoldemPlayerState::OnRep_CurrentChips()
{
	// UE_LOG(LogTemp, Warning, TEXT("[%s] OnRep_CurrentChips: %d"),
	// 		 *GetPlayerName(), CurrentChips);
	
	OnCurrentChipsChanged.Broadcast(CurrentChips);
}

void AHoldemPlayerState::SetCurrentChips(int32 NewChips)
{
	// UE_LOG(LogTemp, Warning, TEXT("[%s] SetCurrentChips: %d (HasAuthority: %d)"),
	// 		  *GetPlayerName(), NewChips, HasAuthority());
	
	CurrentChips = NewChips;
	OnCurrentChipsChanged.Broadcast(CurrentChips);
}

void AHoldemPlayerState::OnRep_CurrentBet()
{
	OnBettingInfoChanged.Broadcast(CurrentBet, TotalBet);
}

void AHoldemPlayerState::OnRep_TotalBet()
{
	OnBettingInfoChanged.Broadcast(CurrentBet, TotalBet);
}

void AHoldemPlayerState::SetCurrentBet(int32 NewBet)
{
	CurrentBet = NewBet;
	OnBettingInfoChanged.Broadcast(CurrentBet, TotalBet);
}

void AHoldemPlayerState::SetTotalBet(int32 NewTotal)
{
	TotalBet = NewTotal;
	OnBettingInfoChanged.Broadcast(CurrentBet, TotalBet);
}

void AHoldemPlayerState::Server_SetSelectedItem_Implementation(EItemType NewItem)
{
	SelectedItem = NewItem;

	UE_LOG(LogTemp, Warning, TEXT("Player %s selected item: %d"),
		*GetPlayerName(), (int32)NewItem);
}
