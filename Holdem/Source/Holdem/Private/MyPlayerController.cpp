// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "MyPlayer.h"
#include "Components/InteractableComponent.h"
#include "Game/HoldemGameState.h"
#include "UI/BarWidget.h"

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Enhanced Input Subsystem에 Input Mapping Context 추가
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (InputMappingContext)
		{
			Subsystem->AddMappingContext(InputMappingContext, 0);
		}
	}
}

void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Enhanced Input Component로 캐스팅
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMyPlayerController::Look);

		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AMyPlayerController::Grab);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, this, &AMyPlayerController::Release);

		EnhancedInputComponent->BindAction(RotateAction, ETriggerEvent::Triggered, this, &AMyPlayerController::RotateHoldingItem);

		EnhancedInputComponent->BindAction(ItemAction, ETriggerEvent::Started, this, &AMyPlayerController::ShowItemWidget);
	}
}

void AMyPlayerController::Look(const FInputActionValue& Value)
{
	// 2D Axis 값 가져오기
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	// 폰이 있는 경우 회전 입력 추가
	if (AMyPlayer* MyPlayer = Cast<AMyPlayer>(GetPawn()))
	{
		FRotator CurrentRotation = GetControlRotation();

		FRotator NewRotation = CurrentRotation;
		NewRotation.Yaw += LookAxisVector.X;
		NewRotation.Pitch += LookAxisVector.Y;

		// 각도 제한 - Pitch(상하)만 제한
		NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch, MinLook, MaxLook);
		
		MyPlayer->LookPitch = NewRotation.Pitch;
		
		SetControlRotation(NewRotation);
	}
}

void AMyPlayerController::Grab(const FInputActionValue& Value)
{
	Server_Grab();
}

void AMyPlayerController::Server_Grab_Implementation()
{
	AMyPlayer* MyPlayer = Cast<AMyPlayer>(GetPawn());
	if (MyPlayer)
	{
		MyPlayer->TryPickUp();
	}
}

void AMyPlayerController::Release(const FInputActionValue& Value)
{
	Server_Release();
}

void AMyPlayerController::RotateHoldingItem(const FInputActionValue& Value)
{
	const float v = Value.Get<float>();
	
	if (FMath::Abs(v) > 0.01f)
	{
		Server_RotateHoldingItem(v * RotationSpeed);
	}
}

void AMyPlayerController::ShowItemWidget(const FInputActionValue& Value)
{
	// Waiting phase 일 때만 아이템 위젯 표시
	AHoldemGameState* GS = Cast<AHoldemGameState>(GetWorld()->GetGameState());
	if (GS && GS->CurrentPhase != EHoldemPhase::Waiting) return;
	
	AMyPlayer* MyPlayer = Cast<AMyPlayer>(GetPawn());
	if (!MyPlayer) return;
	
	if (!bItemWidgetOn)
	{
		MyPlayer->BarWidget->AddToViewport();
		bItemWidgetOn = true;

		// 마우스 커서 켜기
		GetWorld()->GetFirstPlayerController()->SetShowMouseCursor(true);
		//SetInputMode(FInputModeUIOnly());
	}
	else
	{
		MyPlayer->BarWidget->RemoveFromParent();
		bItemWidgetOn = false;

		// 마우스 커서 끄기
		GetWorld()->GetFirstPlayerController()->SetShowMouseCursor(false);
		//SetInputMode(FInputModeGameOnly());
	}
}

void AMyPlayerController::Server_RotateHoldingItem_Implementation(float Value)
{
	AMyPlayer* MyPlayer = Cast<AMyPlayer>(GetPawn());
	if (MyPlayer && MyPlayer->HoldingInteractable)
	{
		AActor* HoldingActor = MyPlayer->HoldingInteractable->GetOwner();

		FRotator CurrentRotation = HoldingActor->GetActorRotation();
		CurrentRotation.Roll += Value;
		HoldingActor->SetActorRotation(CurrentRotation);
	}
}

void AMyPlayerController::Server_Release_Implementation()
{
	AMyPlayer* MyPlayer = Cast<AMyPlayer>(GetPawn());
	if (MyPlayer)
	{
		MyPlayer->TryDrop();
	}
}

