// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MyPlayerWidget.h"

#include "Components/TextBlock.h"
#include "Game/HoldemGameState.h"
#include "Kismet/GameplayStatics.h"

void UMyPlayerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GS = Cast<AHoldemGameState>(UGameplayStatics::GetGameState(this));
}

void UMyPlayerWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	UpdateTimer();
}

void UMyPlayerWidget::UpdateTimer()
{
	if (!GS) return;
	
	if (Txt_Timer)
	{
		Txt_Timer->SetText(FText::FromString(
			FString::Printf(TEXT("%d"), (int32)GS->WaitingTimeRemaining)));
	}
}
