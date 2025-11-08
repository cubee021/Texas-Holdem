// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MyPlayer.generated.h"

UCLASS()
class HOLDEM_API AMyPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyPlayer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UCameraComponent* Camera;

	// 들고 있는 물체를 고정시킬 위치
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USceneComponent* HoldPosition;

public:
	// UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> PlayerWidgetClass;

	UPROPERTY()
	class UMyPlayerWidget* PlayerWidget;

protected:
	// Interaction
	// Line Trace 최대 거리
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float InteractDistance = 1000.0f;

public:
	UPROPERTY()
	class UInteractableComponent* HoldingInteractable;
	
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void TryPickUp();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void TryDrop();
	
	UInteractableComponent* DetectInteractable();
};
