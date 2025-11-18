// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"

/**
 *
 */
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
	// Look
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* LookAction;
	
	UPROPERTY(EditDefaultsOnly, Category="Look")
	float MaxLook = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category="Look")
	float MinLook = -50.0f;
	
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
	// Item
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* ItemAction;
	// 위젯 on/off
	bool bItemWidgetOn = false;
	
	UFUNCTION()
	void ShowItemWidget(const FInputActionValue& Value);
};
