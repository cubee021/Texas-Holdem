// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BarWidget.h"

#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/TextBlock.h"
#include "Game/HoldemGameInstance.h"
#include "Game/HoldemPlayerState.h"

void UBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 체크박스 바인딩
	if (CkBox_Cocktails1)
	{
		CkBox_Cocktails1->OnCheckStateChanged.AddDynamic(this, &UBarWidget::OnCheck_Cocktails1);
	}

	if (CkBox_Cocktails2)
	{
		CkBox_Cocktails2->OnCheckStateChanged.AddDynamic(this, &UBarWidget::OnCheck_Cocktails2);
	}

	if (CkBox_Whiskey1)
	{
		CkBox_Whiskey1->OnCheckStateChanged.AddDynamic(this, &UBarWidget::OnCheck_Whiskey1);
	}

	if (CkBox_Cigar)
	{
		CkBox_Cigar->OnCheckStateChanged.AddDynamic(this, &UBarWidget::OnCheck_Cigar);
	}

	if (CkBox_None)
	{
		CkBox_None->OnCheckStateChanged.AddDynamic(this, &UBarWidget::OnCheck_None);
	}
	
	// 버튼 바인딩
	if (Btn_Back)
	{
		Btn_Back->OnClicked.AddDynamic(this, &UBarWidget::OnClicked_Back);	
	}

	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		AHoldemPlayerState* PS = PC->GetPlayerState<AHoldemPlayerState>();
		if (PS)
		{
			// 초기값 : None
			if(CkBox_None)
			{
				CkBox_None->SetCheckedState(ECheckBoxState::Checked);
				PS->Server_SetSelectedItem(EItemType::None);
			}
			// 플레이어 이름
			if (Txt_Name)
			{
				Txt_Name->SetText(FText::FromString(PS->GetPlayerName()));
			}
		}
	}
}

void UBarWidget::OnCheck_Cocktails1(bool bIsChecked)
{
	if (bIsChecked)
	{
		UpdateCheckBoxSelection(CkBox_Cocktails1);

		APlayerController* PC = GetOwningPlayer();
		if (PC)
		{
			AHoldemPlayerState* PS = PC->GetPlayerState<AHoldemPlayerState>();
			if (PS)
			{
				PS->Server_SetSelectedItem(EItemType::Cocktails);
			}
		}
	}
}

void UBarWidget::OnCheck_Cocktails2(bool bIsChecked)
{
	if (bIsChecked)
	{
		UpdateCheckBoxSelection(CkBox_Cocktails2);

		APlayerController* PC = GetOwningPlayer();
		if (PC)
		{
			AHoldemPlayerState* PS = PC->GetPlayerState<AHoldemPlayerState>();
			if (PS)
			{
				PS->Server_SetSelectedItem(EItemType::Cocktails);
			}
		}
	}
}

void UBarWidget::OnCheck_Whiskey1(bool bIsChecked)
{
	if (bIsChecked)
	{
		UpdateCheckBoxSelection(CkBox_Whiskey1);

		APlayerController* PC = GetOwningPlayer();
		if (PC)
		{
			AHoldemPlayerState* PS = PC->GetPlayerState<AHoldemPlayerState>();
			if (PS)
			{
				PS->Server_SetSelectedItem(EItemType::Whiskey);
			}
		}
	}
}

void UBarWidget::OnCheck_Cigar(bool bIsChecked)
{
	if (bIsChecked)
	{
		UpdateCheckBoxSelection(CkBox_Cigar);

		APlayerController* PC = GetOwningPlayer();
		if (PC)
		{
			AHoldemPlayerState* PS = PC->GetPlayerState<AHoldemPlayerState>();
			if (PS)
			{
				PS->Server_SetSelectedItem(EItemType::Cigar);
			}
		}
	}
}

void UBarWidget::OnCheck_None(bool bIsChecked)
{
	if (bIsChecked)
	{
		UpdateCheckBoxSelection(CkBox_None);

		APlayerController* PC = GetOwningPlayer();
		if (PC)
		{
			AHoldemPlayerState* PS = PC->GetPlayerState<AHoldemPlayerState>();
			if (PS)
			{
				PS->Server_SetSelectedItem(EItemType::None);
			}
		}
	}
}

void UBarWidget::OnClicked_Back()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	UHoldemGameInstance* GI = Cast<UHoldemGameInstance>(GetGameInstance());
	if (!GI) return;
	
	if (PC->HasAuthority())
	{
		// 서버: 세션 파괴 
		GI->DestroyMySession();
	}
	else
	{
		// 클라이언트: 세션 떠나기
		GI->LeaveSession();
	}
}

void UBarWidget::UpdateCheckBoxSelection(class UCheckBox* SelectedCheckBox)
{
	// 체크된 하나 빼고 전부 체크 해제
	CkBox_Cocktails1->SetCheckedState(
		CkBox_Cocktails1 == SelectedCheckBox ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);

	CkBox_Cocktails2->SetCheckedState(
		CkBox_Cocktails2 == SelectedCheckBox ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);

	CkBox_Whiskey1->SetCheckedState(
		CkBox_Whiskey1 == SelectedCheckBox ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);

	CkBox_Cigar->SetCheckedState(
		CkBox_Cigar == SelectedCheckBox ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);

	CkBox_None->SetCheckedState(
		CkBox_None == SelectedCheckBox ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);
}
