// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Game/HandEvaluator.h"
#include "ResultWidget.generated.h"

/**
 * 
 */
 
USTRUCT(BlueprintType)
struct FResultImageData : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UImage* ResultImage;
};
 
UCLASS()
class HOLDEM_API UResultWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Txt_Winner;
	
	UPROPERTY(meta = (BindWidget))
	class UImage* Img_HighestRank;

	void UpdateResult(FString WinnerNames, EHandRank HighestRank);
};
