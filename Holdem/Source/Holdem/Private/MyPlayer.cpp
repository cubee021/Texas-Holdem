// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayer.h"

#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Components/InteractableComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "UI/MyPlayerWidget.h"

// Sets default values
AMyPlayer::AMyPlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// SpringArm 생성 및 설정
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);

	// Camera 생성 및 SpringArm에 부착
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	// HoldPosition 생성 및 Camera에 부착
	HoldPosition = CreateDefaultSubobject<USceneComponent>(TEXT("HoldPosition"));
	HoldPosition->SetupAttachment(Camera);
	HoldPosition->SetRelativeLocation(FVector(100.f, 0.f, 0.f));

	// Default settings
	SpringArm->SetRelativeLocation(FVector(0.000000,0.000000,40.000000));
	SpringArm->TargetArmLength = 300.0f;
	SpringArm->bUsePawnControlRotation = true;

	HoldingInteractable = nullptr;
	LookRotation = FRotator::ZeroRotator;
}

// Called when the game starts or when spawned
void AMyPlayer::BeginPlay()
{
	Super::BeginPlay();

	// UI 추가
	if (PlayerWidgetClass)
	{
		PlayerWidget = CreateWidget<UMyPlayerWidget>(GetWorld(), PlayerWidgetClass);
		if (PlayerWidget)
		{
			PlayerWidget->AddToViewport();
		}
	}
}

void AMyPlayer::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// LookRotation은 Replicated 변수가 아니므로 여기에 추가하지 않음
}

// Called every frame
void AMyPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 서버에서만 실행: 각 플레이어의 ControlRotation을 Multicast
	if (GetLocalRole() == ROLE_Authority && Controller)
	{
		FRotator CurrentRotation = GetControlRotation();
		Multicast_SetLookRotation(CurrentRotation);
	}
	// 원격 플레이어: LookRotation으로 SpringArm 업데이트
	else if (!IsLocallyControlled() && SpringArm)
	{
		FRotator NewRotation = SpringArm->GetComponentRotation();
		NewRotation.Pitch = LookRotation.Pitch;
		SpringArm->SetWorldRotation(NewRotation);
	}
}

void AMyPlayer::Multicast_SetLookRotation_Implementation(FRotator NewRotation)
{
	// Owning Client는 이미 로컬에서 회전하고 있으므로 업데이트 안 함
	if (!IsLocallyControlled())
	{
		LookRotation = NewRotation;
	}
}

// Called to bind functionality to input
void AMyPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AMyPlayer::TryPickUp()
{
	if (HoldingInteractable) return;

	// Ray trace로 InteractableComponent 찾기   
	UInteractableComponent* FoundInteractable = DetectInteractable();
	if (FoundInteractable && HoldPosition)
	{
		FoundInteractable->HoldingOwner = this;
		FoundInteractable->PickUp();

		// 현재 들고 있는 물체로 저장
		HoldingInteractable = FoundInteractable;
	}
}

void AMyPlayer::TryDrop()
{
	if (HoldingInteractable)
	{
		HoldingInteractable->Drop();
		
		HoldingInteractable->HoldingOwner = nullptr;
		HoldingInteractable = nullptr;
	}
}

UInteractableComponent* AMyPlayer::DetectInteractable()
{
	FVector CameraLocation = Camera->GetComponentLocation();
	FVector CameraForward = Camera->GetForwardVector();

	FVector TraceStart = CameraLocation;
	FVector TraceEnd = TraceStart + (CameraForward * InteractDistance);

	// Hit 결과
	FHitResult HitResult;

	// Ray trace 실행
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult, TraceStart, TraceEnd, ECC_Visibility);
	
	DrawDebugLine(GetWorld(), TraceStart, TraceEnd, bHit?FColor::Green : FColor::Red,
		false, 0.1f, 0, 2.0f);

	// Hit한 경우
	if (bHit && HitResult.GetActor())
	{
		// Actor에서 InteractableComponent 찾기
		UInteractableComponent* InteractComp =
			HitResult.GetActor()->FindComponentByClass<UInteractableComponent>();

		if (InteractComp) return InteractComp;
	}

	return nullptr;
}

