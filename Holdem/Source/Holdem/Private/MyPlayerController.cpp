// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

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
		// Look Action 바인딩
		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMyPlayerController::Look);
		}
	}
}

void AMyPlayerController::Look(const FInputActionValue& Value)
{
	// 2D Axis 값 가져오기
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	// 폰이 있는 경우 회전 입력 추가
	if (APawn* ControlledPawn = GetPawn())
	{
		FRotator CurrentRotation = GetControlRotation();

		FRotator NewRotation = CurrentRotation;
		NewRotation.Yaw += LookAxisVector.X;
		NewRotation.Pitch += LookAxisVector.Y;

		// 각도 제한 - Pitch(상하)만 제한
		NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch, MinLook, MaxLook);

		SetControlRotation(NewRotation);
	}
}
