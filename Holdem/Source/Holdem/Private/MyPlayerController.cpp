// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "MyPlayer.h"
#include "Components/InteractableComponent.h"
#include "Game/HoldemGameMode.h"
#include "Game/HoldemGameState.h"
#include "Game/HoldemPlayerState.h"
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

	AHoldemGameState* GS = Cast<AHoldemGameState>(GetWorld()->GetGameState());
	if (GS)
	{
		GS->OnPhaseChanged.AddDynamic(this, &AMyPlayerController::OnGamePhaseChanged);
	}

	SetShowMouseCursor(false);
	SetInputMode(FInputModeGameOnly());
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

		EnhancedInputComponent->BindAction(UpAction, ETriggerEvent::Started, this, &AMyPlayerController::OnBettingUp);
		EnhancedInputComponent->BindAction(DownAction, ETriggerEvent::Started, this, &AMyPlayerController::OnBettingDown);
		EnhancedInputComponent->BindAction(ConfirmAction, ETriggerEvent::Started, this, &AMyPlayerController::OnBettingConfirm);
	}
}

void AMyPlayerController::Look(const FInputActionValue& Value)
{
	// 위젯이 열려있으면 카메라 회전 막기
	if (IsItemWidgetOpen()) return;

	// Look 입력이 비활성화되어 있으면 차단
	if (const AHoldemGameState* GS = GetWorld()->GetGameState<AHoldemGameState>())
	{
		if (GS->bIsLookDisabled) return;
	}

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

void AMyPlayerController::OnGamePhaseChanged(EHoldemPhase NewPhase)
{
	// Waiting이 아닌 다른 페이즈로 전환되면 BarWidget 닫기 
	if ((NewPhase != EHoldemPhase::Waiting) && IsItemWidgetOpen())
	{
		UE_LOG(LogTemp, Warning, TEXT("[PlayerController] Close Widget"));
		CloseItemWidget();
	}
}

void AMyPlayerController::OnBettingUp(const FInputActionValue& Value)
{
	// 위로 이동
	SelectedButtonIndex = (SelectedButtonIndex - 1 + 3)%3;

	OnBettingSelectionChanged.Broadcast(SelectedButtonIndex);
}

void AMyPlayerController::OnBettingDown(const FInputActionValue& Value)
{
	// 아래로 이동
	SelectedButtonIndex = (SelectedButtonIndex + 1)%3;

	OnBettingSelectionChanged.Broadcast(SelectedButtonIndex);
}

void AMyPlayerController::OnBettingConfirm(const FInputActionValue& Value)
{
	AHoldemPlayerState* PS = GetPlayerState<AHoldemPlayerState>();
	AHoldemGameState* GS = GetWorld()->GetGameState<AHoldemGameState>();
	if (!PS || !GS) return;
	
	// 현재 상황 판단                                                                                               
	bool bNoBets = (GS->CurrentMaxBet == 0);
	bool bCanCheck = (PS->CurrentBet == GS->CurrentMaxBet);

	// 선택된 버튼에 따라 RPC 호출                                                                                                                                 
	switch (SelectedButtonIndex)
	{
	case 0:  // Button 0: Raise or Bet                                                                                                                              
		Server_Raise();
		UE_LOG(LogTemp, Warning, TEXT("[OnBettingConfirm] Raise"));
		break;

	case 1:  // Button 1: Call or Check                                                                                                                             
		if (bCanCheck)
		{
			Server_Check();
			UE_LOG(LogTemp, Warning, TEXT("[OnBettingConfirm] Check"));
		}
		else                                                                                                                                                    
		{
			Server_Call();
			UE_LOG(LogTemp, Warning, TEXT("[OnBettingConfirm] Call"));
		}
		break;

	case 2:  // Button 2: Fold                                                                                                                                      
		Server_Fold();
		UE_LOG(LogTemp, Warning, TEXT("[OnBettingConfirm] Fold"));
		break;
	}
}

void AMyPlayerController::Server_Check_Implementation()
{
	if (!IsMyTurn()) return;
	
	AHoldemGameMode* GM = GetWorld()->GetAuthGameMode<AHoldemGameMode>();
	AHoldemGameState* GS = GetWorld()->GetGameState<AHoldemGameState>();
	AHoldemPlayerState* PS = GetPlayerState<AHoldemPlayerState>();
	if (!GM || !GS || !PS) return;

	// Check 가능한지 확인
	if (PS->CurrentBet != GS->CurrentMaxBet)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Server_Check] Cannot check! Must Call or Fold."));
		return;
	}

	// 체크 처리 후, 다음 플레이어로 이동
	GS->ProcessCheck(PS);
	GM->MoveToNextPlayer();
}

void AMyPlayerController::Server_Fold_Implementation()
{
	if (!IsMyTurn()) return;
	
	AHoldemGameMode* GM = GetWorld()->GetAuthGameMode<AHoldemGameMode>();
	AHoldemGameState* GS = GetWorld()->GetGameState<AHoldemGameState>();
	AHoldemPlayerState* PS = GetPlayerState<AHoldemPlayerState>();
	if (!GM || !GS || !PS) return;

	// 폴드 처리 후, 다음 플레이어로 이동
	GS->ProcessFold(PS);
	GM->MoveToNextPlayer();
}

void AMyPlayerController::Server_Call_Implementation()
{
	if (!IsMyTurn()) return;
	
	AHoldemGameMode* GM = GetWorld()->GetAuthGameMode<AHoldemGameMode>();
	AHoldemGameState* GS = GetWorld()->GetGameState<AHoldemGameState>();
	AHoldemPlayerState* PS = GetPlayerState<AHoldemPlayerState>();
	if (!GM || !GS || !PS) return;

	// 충분한 칩이 있는지 확인
	int32 CallAmount = GS->CurrentMaxBet - PS->CurrentBet;
	if (PS->CurrentChips < CallAmount)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Server_Call] Not enough chips!"));
		return;
	}

	// 콜 처리 후, 다음 플레이어로 이동
	GS->ProcessCall(PS);
	GM->MoveToNextPlayer();
}

void AMyPlayerController::Server_Raise_Implementation()
{
	if (!IsMyTurn()) return;
	
	AHoldemGameMode* GM = GetWorld()->GetAuthGameMode<AHoldemGameMode>();
	AHoldemGameState* GS = GetWorld()->GetGameState<AHoldemGameState>();
	AHoldemPlayerState* PS = GetPlayerState<AHoldemPlayerState>();
	if (!GM || !GS || !PS) return;
	
	// 충분한 칩이 있는지 확인
	int32 RaiseAmount = GS->BigBlindAmount * 2;
	if (PS->CurrentChips < RaiseAmount)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Server_Raise] Not enough chips!"));
		return;
	}

	// 레이즈 처리 후, 다음 플레이어로 이동
	GS->ProcessRaise(PS);
	GM->MoveToNextPlayer();
}

bool AMyPlayerController::IsMyTurn() const
{
	AHoldemGameState* GS = GetWorld()->GetGameState<AHoldemGameState>();
	AHoldemPlayerState* PS = GetPlayerState<AHoldemPlayerState>();
	if (!GS || !PS) return false;

	int32 MyIndex = GS->PlayerArray.Find(PS);
	if (MyIndex == INDEX_NONE) return false;
	
	return GS->CurrentTurnPlayerIndex == MyIndex;
}

void AMyPlayerController::ShowItemWidget(const FInputActionValue& Value)
{
	// Waiting phase 일 때만 아이템 위젯 표시
	AHoldemGameState* GS = Cast<AHoldemGameState>(GetWorld()->GetGameState());
	if (GS && GS->CurrentPhase != EHoldemPhase::Waiting) return;
	
	AMyPlayer* MyPlayer = Cast<AMyPlayer>(GetPawn());
	if (!MyPlayer) return;
	
	if (!IsItemWidgetOpen())
	{
		// 처음 한 번만 AddToViewport
		if (!MyPlayer->BarWidget->IsInViewport())
		{
			MyPlayer->BarWidget->AddToViewport();
		}

		// 보이기
		MyPlayer->BarWidget->SetVisibility(ESlateVisibility::Visible);

		// 마우스 커서 켜기
		SetShowMouseCursor(true);
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
	}
	else
	{
		CloseItemWidget();
	}
}

void AMyPlayerController::CloseItemWidget()
{
	AMyPlayer* MyPlayer = Cast<AMyPlayer>(GetPawn());
	if (!MyPlayer) return;
	
	// RemoveFromParent 대신 숨기기
	MyPlayer->BarWidget->SetVisibility(ESlateVisibility::Hidden);

	// 마우스 커서 끄기
	SetShowMouseCursor(false);
	SetInputMode(FInputModeGameOnly());
}

bool AMyPlayerController::IsItemWidgetOpen() const
{
	AMyPlayer* MyPlayer = Cast<AMyPlayer>(GetPawn());
	if (!MyPlayer || !MyPlayer->BarWidget) return false;
	
	return MyPlayer->BarWidget->GetVisibility() == ESlateVisibility::Visible;
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

