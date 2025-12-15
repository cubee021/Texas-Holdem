// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"

/**
 *
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBettingSelectionChanged, int32, SelectedIndex);

UCLASS()
class HOLDEM_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

public:
	// Input Mapping Context
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputMappingContext* InputMappingContext;

protected:
	//---------------------------------------------------//
	// Basic Input
	//---------------------------------------------------//
	
	// Look
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* LookAction;
	
	UPROPERTY(EditDefaultsOnly, Category="Look")
	float MaxLook = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category="Look")
	float MinLook = -60.0f;
	
	UFUNCTION()
	void Look(const FInputActionValue& Value);

protected:
	// Interact
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* InteractAction;
	
	UFUNCTION()
	void Grab(const FInputActionValue& Value);

	UFUNCTION(Server, Reliable)
	void Server_Grab();
	
	UFUNCTION()
	void Release(const FInputActionValue& Value);

	UFUNCTION(Server, Reliable)
	void Server_Release();

protected:
	// Rotate
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* RotateAction;

	UPROPERTY(EditAnywhere, Category = "Rotation")
	float RotationSpeed = 30.f;
	
	UFUNCTION()
	void RotateHoldingItem(const FInputActionValue& Value);

	UFUNCTION(Server, Reliable)
	void Server_RotateHoldingItem(float Value);

protected:
	// Phase deligate
	UFUNCTION()
	void OnGamePhaseChanged(EHoldemPhase NewPhase);

protected:
	//---------------------------------------------------//
	// Betting Input
	//---------------------------------------------------//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* UpAction; // W키

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* DownAction; // S키

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* ConfirmAction; // E키
	
	UFUNCTION()
	void OnBettingUp(const FInputActionValue& Value);

	UFUNCTION()
	void OnBettingDown(const FInputActionValue& Value);

	UFUNCTION()
	void OnBettingConfirm(const FInputActionValue& Value);

public:
	// 현재 선택된 버튼 인덱스 (0, 1, 2)
	int32 SelectedButtonIndex = 1;

	// 중복 입력 방지용 타이머
	float LastConfirmTime = 0.f;
	
public:
	// 델리게이트: 버튼 선택 변경 시 UI에 알림
	UPROPERTY(BlueprintAssignable)
	FOnBettingSelectionChanged OnBettingSelectionChanged;

public:
	//---------------------------------------------------//
	// Betting RPC
	//---------------------------------------------------//
	UFUNCTION(Server, Reliable)
	void Server_Fold();

	UFUNCTION(Server, Reliable)
	void Server_Check();

	UFUNCTION(Server, Reliable)
	void Server_Call();

	UFUNCTION(Server, Reliable)
	void Server_Raise();

	// 현재 내 차례인지 체크
	bool IsMyTurn() const;
	
protected:
	//---------------------------------------------------//
	// Item
	//---------------------------------------------------//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* ItemAction;
	
	UFUNCTION()
	void ShowItemWidget(const FInputActionValue& Value);
	void CloseItemWidget();

	bool IsItemWidgetOpen() const;
	
};
