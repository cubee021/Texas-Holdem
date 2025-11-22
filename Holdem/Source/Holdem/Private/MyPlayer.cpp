// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayer.h"

#include "MyPlayerSaveGame.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Components/InteractableComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "UI/BarWidget.h"
#include "UI/MyPlayerWidget.h"
#include "UI/NameTagWidget.h"

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

	// NameTag 생성
	NameTag = CreateDefaultSubobject<UWidgetComponent>(TEXT("NameTag"));
	NameTag->SetupAttachment(GetRootComponent());
	
	// Default settings
	SpringArm->SetRelativeLocation(FVector(0.000000,0.000000,40.000000));
	SpringArm->TargetArmLength = 300.0f;
	SpringArm->bUsePawnControlRotation = true;

	HoldingInteractable = nullptr;
	LookPitch = 0.f;
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

	if (BarWidgetClass)
	{
		BarWidget = CreateWidget<UBarWidget>(GetWorld(), BarWidgetClass);
		IsValid(BarWidget);
	}
	
	// NameTag 업데이트
	UNameTagWidget* Tag = Cast<UNameTagWidget>(NameTag->GetWidget());
	if (Tag)
	{
		UMyPlayerSaveGame* SG = Cast<UMyPlayerSaveGame>(
		UGameplayStatics::LoadGameFromSlot(TEXT("PlayerDataSlot"), 0));

		if (!SG) return;
		
		Tag->SetName(SG->PlayerName);
	}
}

void AMyPlayer::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyPlayer, LookPitch);
}

void AMyPlayer::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	Client_OnPossess();
}

// Called every frame
void AMyPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	BillboardNameTag();
}

// Called to bind functionality to input
void AMyPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AMyPlayer::OnRep_LookPitch()
{
	if (SpringArm)
	{
		// 다른 클라이언트에서 보이는 서버 캐릭터의 복제본의 설정 변경
		SpringArm->bUsePawnControlRotation = false;

		FRotator CurrentRotation = SpringArm->GetRelativeRotation();
		CurrentRotation.Pitch = LookPitch;
		SpringArm->SetRelativeRotation(CurrentRotation);
	}
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

void AMyPlayer::BillboardNameTag()
{
	// 내가 컨트롤하고 있는 카메라를 가져오자.
	AActor* cam = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	// 카메라의 앞 방향 (반대), 윗 방향을 이용해서 Rotator 를 구하자.
	FRotator rot = UKismetMathLibrary::MakeRotFromXZ(-cam->GetActorForwardVector(), cam->GetActorUpVector());
	// 구한 Rotator 를 comHP 에 설정
	NameTag->SetWorldRotation(rot);
}

void AMyPlayer::Client_OnPossess_Implementation()
{
	NameTag->SetVisibility(false);
}

