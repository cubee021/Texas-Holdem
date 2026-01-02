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
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PossessedBy(AController* NewController) override;
	
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
	// 플레이어 이름 표시
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UWidgetComponent* NameTag;

public:
	// UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> PlayerWidgetClass;

	UPROPERTY()
	class UMyPlayerWidget* PlayerWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> BarWidgetClass;

	UPROPERTY()
	class UBarWidget* BarWidget;

protected:
	// Interaction
	// Line Trace 최대 거리
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float InteractDistance = 500.0f;

public:
	// 서버쪽 pitch 수동으로 동기화
	// bUsePawnControlRotation은 서버->클라로 전달 안됨
	UPROPERTY(ReplicatedUsing=OnRep_LookPitch)
	float LookPitch;

	UFUNCTION()
	void OnRep_LookPitch();

public:
	UPROPERTY()
	class UInteractableComponent* HoldingInteractable;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void TryPickUp();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void TryDrop();

	UInteractableComponent* DetectInteractable();

protected:
	// NameTag
	void BillboardNameTag();

	// Steam 정보 업데이트용 타이머
	FTimerHandle NameTagTimerHandle;
	void TryUpdateNameTag();

	UFUNCTION(Client, Reliable)
	void Client_OnPossess();
};
